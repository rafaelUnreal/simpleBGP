# simpleBGP
Implementation of BGP protocol from scratch for study purposes.

# TODO
- Idle/Connect/Active/OpenConfirm/Established states(in progress)
    - OpenConfirm state(completed)
    - Established state(completed)
    - Active state(completed)
    
- parse open message(completed)
- parse keepalive(completed)
- send keepalive message(completed)
- keepalive and hold_time events(completed)

- BGP RIB using prefix trie(completed)
- Route updates
  - Send local update advertisment(completed)
  - Receive route updates(in progress)
  - Send advertised updates(in progress)
  - Send withdrawn updates(in progress)
  
- BGP session initiator
- Install best path from BGP RIB in GNU/Linux FIB
- BGP Notifications
- BGP Daemon

# Log Output
```bash
Simple BGP. Author Rafael P.
poll()
BGP_new_connection() from 192.168.0.102:37780
ACTIVE
poll()
recv()
BGP_event_receive(): neighbor: 192.168.0.102
BGP_active()
BGP Open
BGP Open ASN=1DFB
BGP Open Hold Time=B4
BGP Open Version=04
BGP_open_message_check()
config_get_neighbor()
configGetNeighbor().cfg_ip == ip_address; elem = 0
config_get_asn()
config_get_version()
config_get_version()
config_get_asn()
config_get_hold_time()
config_get_id()
starting bgp keepalive_event()
starting bgp hold_time_event()
ACTIVE-> OPEN_CONFIRM
BGP_reply to be send; sendAll()
sendAll()
BGP_event_send(): neighbor: 192.168.0.102
round robin=1
poll()
recv()
BGP_event_receive(): neighbor: 192.168.0.102
BGP_open_confirm()
BGP_open_confirm(); BGP keepalive
OPEN_CONFIRM -> ESTABLISHED
BGP_reply to be send; sendAll()
sendAll()
BGP_event_send(): neighbor: 192.168.0.102
BGP_established(): neighbor: 192.168.0.102
```
