### **Exp 1: Configuring an IP Network**

#### **Steps**

1. **Connect the devices:**
   - Connect `E1` of `tux43` and `E1` of `tux44` to switches 3 and 4.

2. **Configure IP addresses on Tux 3 and Tux 4:**

   - On **Tux 3**:
     ```bash
     ifconfig eth1 up
     ifconfig eth1 172.16.40.1/24
     ```

   - On **Tux 4**:
     ```bash
     ifconfig eth1 up
     ifconfig eth1 172.16.40.254/24
     ```

3. **Record the IP and MAC addresses:**
   - Use `ifconfig` to view the IP and MAC addresses of the network interfaces.
     ```bash
     ifconfig
     ```
   - Look for the `ether` field for the MAC address.

   LOG DO IFCONFIG DE CADA TUX

4. **Verify network connectivity:**
   - On **Tux 3**, ping **Tux 4**:
     ```bash
     ping 172.16.40.254
     ```
   - On **Tux 4**, ping **Tux 3**:
     ```bash
     ping 172.16.40.1
     ```

5. **Inspect Forwarding and ARP Tables:**
   - **Check routing table:**
     ```bash
     route -n
     ```

     ```bash
     Kernel IP routing table
     Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
     0.0.0.0         10.227.20.254   0.0.0.0         UG    0      0        0 eth0
     10.227.20.0     0.0.0.0         255.255.255.0   U     0      0        0 eth0
     172.16.40.0     0.0.0.0         255.255.255.0   U     0      0        0 eth1
     ```

   - **Check ARP table:**
     ```bash
     arp -a
     ```

     ```bash
     root@gnu43:~# arp -a
     ? (172.16.40.254) at 00:c0:df:02:55:95 [ether] on eth1
     ? (10.227.20.254) at e4:8d:8c:20:25:c8 [ether] on eth0
     ```

6. **Delete ARP Table Entries on Tux 3:**
   To delete an ARP entry for a specific IP (e.g., `172.16.40.254`), use the command:
   ```bash
   arp -d 172.16.40.254
   ```

7. **Capture Network Traffic with Wireshark:**
   - On **Tux 3**, start Wireshark on interface `eth1` and ping the destination IP:
     ```bash
     ping 172.16.50.254
     ```

---

#### **Questions and Answers**

1. **What are ARP packets and what are they used for?**
   - ARP (Address Resolution Protocol) is used to map an IP address to a MAC address within a local network. ARP packets are sent to request or respond with the MAC address corresponding to a specific IP address.

2. **What are the MAC and IP addresses in ARP packets and why are they important?**
   - ARP packets carry both the source and destination IP addresses. For example, an ARP request may include the source IP (`tux43, 172.16.40.1`) and the destination IP (`tux44, 172.16.40.254`). The response will include the MAC address of the destination IP, allowing the sender to associate the IP address with the corresponding hardware address.

3. **What packets does the ping command generate?**
   - The `ping` command generates ICMP packets after resolving the destination's MAC address. If the MAC address is unknown, it sends an ARP request first.

4. **What are the MAC and IP addresses in the ping packets?**
   - The ping packets include the source and destination IP addresses, which were previously configured in the devices. The source MAC address is that of the machine initiating the ping, and the destination MAC address is resolved through ARP.

5. **How can you determine if a received Ethernet frame is ARP, IP, or ICMP?**
   - Analyzing the Protocol column in the Wireshark capture.

6. **How can you determine the length of a received frame?**
   - Analyzing the Length column in the Wireshark capture.

7. **What is the loopback interface and why is it important?**
   - The loopback interface is a virtual network interface that a device uses to communicate with itself. It's crucial for testing, ensuring that network software is functioning correctly without needing to use physical network devices.

Here is the properly formatted content for **Exp 2: Implement two bridges in a switch**:

---

### **Exp 2: Implement two bridges in a switch**

#### **Steps**

1. **Connect `E1` of `tux42` to switch 2**:
   
   On **Tux 42**:
   ```bash
   ifconfig eth1 up
   ifconfig eth1 172.16.41.1/24
   ```

   - Write down IP and MAC address:
     ```bash
     eth1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
         inet 172.16.41.1  netmask 255.255.255.0  broadcast 172.16.41.255
         inet6 fe80::2e0:7dff:feb5:8c8f  prefixlen 64  scopeid 0x20<link>
         ether 00:e0:7d:b5:8c:8f  txqueuelen 1000  (Ethernet)
         RX packets 24  bytes 3024 (2.9 KiB)
         RX errors 0  dropped 12  overruns 0  frame 0
         TX packets 68  bytes 10713 (10.4 KiB)
         TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
     ```

   - IP: `172.16.41.1`
   - MAC: `00:e0:7d:b5:8c:8f`

2. **In GTK, create two bridges in the switch**:
   ```bash
   /interface bridge add name=bridge40
   /interface bridge add name=bridge41
   ```

3. **Remove the ports from the default bridge and add them to the corresponding bridges**:
   ```bash
   /interface bridge port remove [find interface=ether2] 
   /interface bridge port remove [find interface=ether3] 
   /interface bridge port remove [find interface=ether4] 

   /interface bridge port add bridge=bridge41 interface=ether2
   /interface bridge port add bridge=bridge40 interface=ether3
   /interface bridge port add bridge=bridge40 interface=ether4
   ```

4. **Capture `eth1` of Tux 43 with Wireshark** and ping Tux 44 and Tux 42:
   ```bash
   ping 172.16.40.254 
   ping 172.16.41.1
   ```

   - The ping to Tux 44 should work, while Tux 42 shouldn't respond.

5. **Capture `eth1` of each Tux**:
   - On **Tux 43**, do a broadcast ping:
     ```bash
     ping -b 172.16.40.255
     ```
   - Save the results from each Tux.

6. **Repeat the previous step** but now ping the broadcast address on **Tux 42**:
   ```bash
   ping -b 172.16.41.255
   ```

---

#### **Questions and Answers**

1. **How to configure bridgeY0?**  
   **Bridge40** was configured to connect **Tux43** and **Tux44**, creating a subnet. The default switch configuration was removed, and specific interfaces were assigned to the bridge. This process involves creating the bridge, removing default ports, and assigning the desired interfaces to the bridge, ensuring communication between the computers on the same subnet.

2. **How many broadcast domains are there? How can you conclude it from the logs?**  
   There are two broadcast domains, as two separate bridges are created. From the logs, we observe that **Tux43** can ping **Tux44** successfully, but it can't reach **Tux42**, as **Tux42** is on a different bridge. This is confirmed by the Wireshark captures, which show the communication within the respective domains.

---

### **Exp 3: Configure a Router in Linux**

#### **Questions and Answers**

1. **What routes are there in the tuxes? What are their meaning?**


2. **What information does an entry of the forwarding table contain?**


3. **What ARP messages, and associated MAC addresses, are observed and why?**


4. **What ICMP packets are observed and why?**


5. **What are the IP and MAC addresses associated to ICMP packets and why?** 


---

### **Exp 4: Configure a Commercial Router and Implement NAT**

#### **Questions and Answers**

1. **How to configure a static route in a commercial router?**


2. **What are the paths followed by the packets, with and without ICMP redirect enabled, in the experiments carried out and why?**


3. **How to configure NAT in a commercial router?**


4. **What does NAT do?**


5. **What happens when tuxY3 pings the FTP server with NAT disabled? Why?**


---

### **Exp 5: DNS**

#### **Questions and Answers**

1. **How to configure the DNS service in a host?**


2. **What packets are exchanged by DNS and what information is transported?**
 

---

### **Exp 6: TCP Connections**

#### **Questions and Answers**

1. **How many TCP connections are opened by your FTP application?**


2. **In what connection is transported the FTP control information?**


3. **What are the phases of a TCP connection?**


4. **How does the ARQ TCP mechanism work? What are the relevant TCP fields? What relevant information can be observed in the logs?**
   

5. **How does the TCP congestion control mechanism work? What are the relevant fields? How did the throughput of the data connection evolve along the time? Is it according to the TCP congestion control mechanism?**


6. **Is the throughput of a TCP data connection disturbed by the appearance of a second TCP connection? How?**

