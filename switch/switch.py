#!/usr/bin/python3
import sys
import struct
import wrapper
import threading
import time
from bpdu import SPT, is_multicast, create_packet
from wrapper import recv_from_any_link, send_to_link, get_switch_mac, get_interface_name


def parse_ethernet_header(data):
    # Unpack the header fields from the byte array
    # dest_mac, src_mac, ethertype = struct.unpack('!6s6sH', data[:14])
    dest_mac = data[0:6]
    src_mac = data[6:12]

    # Extract ethertype. Under 802.1Q, this may be the bytes from the VLAN TAG
    ether_type = (data[12] << 8) + data[13]

    vlan_id = -1
    # Check for VLAN tag (0x8100 in network byte order is b'\x81\x00')
    if ether_type == 0x8200:
        vlan_tci = int.from_bytes(data[14:16], byteorder="big")
        vlan_id = vlan_tci & 0x0FFF  # extract the 12-bit VLAN ID
        ether_type = (data[16] << 8) + data[17]

    return dest_mac, src_mac, ether_type, vlan_id


def create_vlan_tag(vlan_id):
    # 0x8100 for the Ethertype for 802.1Q
    # vlan_id & 0x0FFF ensures that only the last 12 bits are used
    return struct.pack("!H", 0x8200) + struct.pack("!H", vlan_id & 0x0FFF)


def send_bdpu_every_sec(stp: SPT, vlan_config: dict, interfaces: range):
    while True:
        if stp.is_root():
            for fa in interfaces:
                # print(f"{fa} with vlan_id {vlan_config[get_interface_name(fa)]}")
                if vlan_config[get_interface_name(fa)] == "T":
                    data = stp.create_packet()
                    send_to_link(fa, data, len(data))

        time.sleep(1)



def is_unicast(mac: str) -> bool:
    return mac.lower() != "ff:ff:ff:ff:ff:ff" and not is_multicast(mac)


def parse_config(switch_id: str, interfaces: range):
    f_in = open(f"configs/switch{switch_id}.cfg", "r")
    priority = int(f_in.readline())

    vlans_config = {}
    for _ in interfaces:
        config = f_in.readline().rstrip("\n").split(" ")
        vlans_config[config[0]] = config[1]

    f_in.close()

    return priority, vlans_config

# VLAN support
def send_vlan_packet(interface, data, length, vlan_config, vlan_id, vlan_mod_enter):
    vlan_out_id = vlan_config[get_interface_name(interface)]

    if vlan_out_id == "T":
        # Receieved from an access point
        if vlan_id == -1:
            data = data[:12] + create_vlan_tag(vlan_mod_enter) + data[12:]
            length = len(data)
            send_to_link(interface, data, length)
        else:
            send_to_link(interface, data, length)
    elif int(vlan_out_id) == vlan_mod_enter:
        # Receieved from an access point
        if vlan_id != -1:
            send_to_link(interface, data[:12] + data[16:], len(data[:12] + data[16:]))
        else:
            send_to_link(interface, data, length)


def main():
    # init returns the max interface number. Our interfaces
    # are 0, 1, 2, ..., init_ret value + 1
    switch_id = sys.argv[1]

    num_interfaces = wrapper.init(sys.argv[2:])
    interfaces = range(0, num_interfaces)

    print("# Starting switch with id {}".format(switch_id), flush=True)
    print("[INFO] Switch MAC", ':'.join(f'{b:02x}' for b in get_switch_mac()))

    CAM_table = {}

    prio, vlan_config = parse_config(switch_id, interfaces)

    stp = SPT(interfaces, prio, vlan_config)

    # Printing interface names
    for i in interfaces:
        name = get_interface_name(i)

    # Create and start a new thread that deals with sending BDPU
    t = threading.Thread(
        target=send_bdpu_every_sec, args=(stp, vlan_config, interfaces)
    )
    t.start()
    while True:
        interface, data, length = recv_from_any_link()

        dest_mac, src_mac, ethertype, vlan_id = parse_ethernet_header(data)

        # Print the MAC src and MAC dst in human readable format
        dest_mac = ":".join(f"{b:02x}" for b in dest_mac)
        src_mac = ":".join(f"{b:02x}" for b in src_mac)

        print(f"Destination MAC: {dest_mac}")
        print(f"Source MAC: {src_mac}")
        print(f"EtherType: {ethertype}")
        print(f"VLAN ID: {vlan_id}")
        print(
            "Received frame of size {} on interface {} - {}".format(length, interface, get_interface_name(interface)),
            flush=True,
        )

        # Put the source in the CAM table
        CAM_table[src_mac] = interface

        # Parses the packet if is bpdu:
        if stp.is_bpdu(interface, dest_mac, data, len):
            continue

        # vlan_id_in should always pe populated. Is the id from where the packet was received 
        vlan_id_in: int = -1
        if vlan_id != -1:
            vlan_id_in = vlan_id
        else:
            vlan_id_in = int(vlan_config[get_interface_name(interface)])
        
        # Forwarding with learning
        if (is_unicast(dest_mac) and
            dest_mac in CAM_table and
            CAM_table[dest_mac] in stp.get_interfaces()
        ):
            send_vlan_packet(
                CAM_table[dest_mac], data, length, vlan_config, vlan_id, vlan_id_in
            )
        else:
            for i in stp.get_interfaces():
                if i != interface:
                    send_vlan_packet(
                        i, data, length, vlan_config, vlan_id, vlan_id_in
                    )


if __name__ == "__main__":
    main()
