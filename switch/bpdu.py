import struct

from scapy.layers.inet import print_function

from wrapper import send_to_link, get_switch_mac, get_interface_name

def is_multicast(mac: str) -> bool:
    octet = int(mac.split(":")[0], 16)
    return (octet & 1) != 0 and mac.lower() != "ff:ff:ff:ff:ff:ff"

def create_packet(root_bridge_ID, sender_bridge_ID, sender_path):
    dest_mac = bytes.fromhex("0180C2000000")
    llc_header = struct.pack("3b", 0x42, 0x42, 0x03)
    bpdu_header = struct.pack("!i", 0)
    bpdu_config = struct.pack("!3i", root_bridge_ID, sender_bridge_ID, sender_path)
    llc_len = len(llc_header + bpdu_header + bpdu_config)
    return (
        dest_mac + 
        get_switch_mac() + 
        struct.pack("!H", llc_len) +
        llc_header + 
        bpdu_header +
        bpdu_config
    )

class SPT:
    def __init__(self, interfaces, priority, vlan_config):
        self.root_bridge_ID = priority
        self.root_path_cost = 0
        self.vlan_config = vlan_config
        self.own_bridge_ID = priority
        self.interfaces = interfaces
        self.interface_state = {}
        self.designated_ports = []
        self.root_port = -1

        for fa in interfaces:
            if self.vlan_config[get_interface_name(fa)] != "T":
                self.interface_state[fa] = "LISTEN"
            else:
                self.interface_state[fa] = "BLOCKED"
                self.designated_ports.append(fa)
    
    def create_packet(self):
        return create_packet(self.own_bridge_ID, self.own_bridge_ID, 0)

    def get_interfaces(self):
        list_ports = []
        for i in self.interfaces:
            if self.interface_state[i] != "BLOCKED":
                list_ports.append(i)
        return list_ports;
    
    def is_root(self):
        return self.own_bridge_ID == self.root_bridge_ID

    def is_bpdu(self, interface, dest_mac, data, len) -> bool:
        if is_multicast(dest_mac):
            self.parse_packet(interface, data, len)
            return True
        return False

    def parse_packet(self, interface, data, len):
        root_bridge_ID, sender_bridge_ID, sender_path = struct.unpack("!3i", data[21:])
        if root_bridge_ID < self.root_bridge_ID:
            self.root_bridge_ID = root_bridge_ID
            self.root_path_cost = sender_path + 10
            self.root_port = interface

            if self.own_bridge_ID == self.root_bridge_ID:
                for fa in self.interfaces:
                    if self.vlan_config[get_interface_name(fa)] == "T" and fa != interface:
                        self.interface_state[fa] = "BLOCKED"
                        self.designated_ports.remove(fa)

            if self.interface_state[self.root_port] == "BLOCKED":
                self.interface_state[self.root_port] = "LISTEN"
            
            data = create_packet(root_bridge_ID, self.own_bridge_ID, self.root_path_cost)
            for fa in self.interfaces:
                if self.vlan_config[get_interface_name(fa)] == "T" and fa != interface:
                    send_to_link(fa, data, len(data))
             
            
        elif root_bridge_ID == self.root_bridge_ID:
            if self.root_port == interface and sender_bridge_ID + 10 < self.root_path_cost:
                self.root_path_cost = sender_path + 10
            elif self.root_port != interface:
                if sender_path > self.root_path_cost:
                    if interface not in self.designated_ports: # posibil aici
                        self.interface_state[interface] = "LISTEN"
                        self.designated_ports.append(interface)
            

        elif sender_bridge_ID == self.own_bridge_ID:
            self.interface_state[interface] = "BLOCKED"
            self.designated_ports.remove(interface)


        if self.own_bridge_ID == self.root_bridge_ID:
            for fa in self.interfaces:
                if fa != self.root_port and self.vlan_config[get_interface_name(fa)] == "T":
                    self.designated_ports.append(fa)
                    self.interface_state[fa] = "LISTEN"

