# Switch

`Author : Rotari Corneliu`
`Group : 333CD`

## Learining

The learning process is done using a dict `CAM_table` to store the known sources.

The pseoudecode from the condition, i extracted there is only one time in which the packet is sent to unicast.

In the rest of the cases i can be abstracted into sending to all other interfaces.

## VLAN

First the switchX.cfg is read to determin the interfaces configured vlan.

Then it checks if the `vlan_id` is -1 then is access point.

After that it looks if the interfaces it leaves is trunk it places a tag.

If the interface is not trunk, it checks if the same vlan is configured.

In the case it is, it removes the tag.

## STP

If the packet is multicast it assumes its intend for stp.

If dest_mac si not in multicast it forwerded.

For the STP algorithm, the bpdu packets are formed only from:

```text
Format:
| dest_mac | src_mac | llc_len | llc_header | bpdu_header | bpdu_config |
```

Where *bpdu_config* is formed from root_bridge_ID, sender_bridge_ID, sender_path_cost.

The desgined ports are represented as a list for an easier adittion and removel.

Every switch implements an personalised object of STP where all the information is stored.
