# Book Notes

## What is the Internet?
The Internet is a computer network that interconnects hundreds of millions of computing devices throughout the world. All of these devices are called **hosts** or **end systems**. <br />
End systems are connected together by a network of **communication links** and **packet switches**. There are many types of communication links, which are made up of different types of physical media, including coaxial cable, copper wire, optical fiber and radio spectrum. Different links can transmit data at different rates, with the **transmission rate** of a link measured in bits/second. When one end system has data to send to another end system, the sending end system segments the data and adds header bytes to each segment. The resulting packages of information, known as **packets** in the jargon of computer networks, are then sent through the network to the destination end system, where they are reassembled into the original data. <br />
End systems access the Internet through **Internet Service Providers (ISPs)**, including local cable or telephone companies, corporate ISPs, university ISPs etc. Each ISP is in itself a network of packet switches and communication links. ISPs provide variety of types of network access to the end systems, including residential broadband access such as cable modem or DSL. <br />
The Internet is all about connecting end systems to each other, so the ISPs that provide access to end systems must also be interconnected. These lower-tier ISPs are interconnected through national and international upper-tier ISPs such as Level 3 Communication, AT&T etc. An upper-tier ISP consists of high-speed routers interconnected with high-speed fiber-optic links. Each ISP network is managed independently, runs the IP protocol and conforms to certain naming and address conventions. <br />
End systems, packet switches, and other pieces of the Internet run **protocols** that control the sending and receiving of information within the Internet. The **Transmission Control Protocol (TCP)** and the **Internet Protocol (IP)** are two of the most important protocols in the Internet. <br />
The IP protocol specifies the format of the packets that are sent and received among routers and end systems. The Internet's principal protocols are collectively known as **TCP/IP**. <br />
**Internet standards** are developed by Internet Engineering Task Force (IETF). The IETF standards documents are called **requests for comments (RCFs)**. They define protocols such as TCP, IP, HTTP and SMTP.

### A Services Description
**Distributed application** involve multiple end systems that exchange data with each other. Internet applications run on end systems - they do not run in the packet switches in the network core. <br />
End systems attached to the Internet provide an **Application Programming Interface (API)** that specifies how a program running on one end system asks the Internet infrastructure to deliver data to a specific destination program running on another end system. This Internet API is a set of rules that the sending program must follow so that the Internet can deliver the data to the destination program.

### What Is a Protocol
It takes two (or more) communcating entities running the same protocol in order to accomplish a task. <br />
TCP Connection request ---> <br />
<--- TCP Connection reply <br />
GET http://www.awl.com/kurose-ross ---> <br />
<--- file <br />

#### Network Protocols
All activity in the Internet that involves two or more communicating remote entities (computer, smartphone, tablet, router etc.) is governed by a protocol.
Hardware-implemented protocols in two physically connected computers control the flow of bits on the "wire" between the two network interface cards; congestion-control protocols in end systems control the rate at which packets are transmitted between sender and receiver; protocols in routers determine a packet's path from source to destination. <br />

## The Network Edge
The computers and other devices connected to the Internet are often referred to as end systems because they sit at the edge of the Internet. The Internet's end systems include desktop computers (PCs, Macs, Linux boxes), servers (e.g. Web servers and Email servers) and mobile computers (e.g.g laptops, smartphones and tablets). <br />
End systems are also referred to as *host* because they host (run) application programs such as a Web broeser program, a Web server program, an e-mail client program or an e-mail server program. Hosts are divided into two categories: **clients** and **servers**. Clients tend to be desktop and mobile PCs, smartphones and so on, whereas servers tend to be more powerful machines that store and distribute Web pages, stream video, relay e-mail and so on. <br /> 
Most of the servers from which we receive search results, e-mail, Web pages, and videos reside in large **data centers** (Google has 30-50 data centers, each having more than one hundred thousand servers). <br />

### Access Networks
Access network is the network that physicaly connects an end system to the first router (edge router) on a path from the end system to any other distant end system. <br />
The two most prelavent types of broadband residential access are **digital subscriber line (DSL)** and **cable**. A residence typically obtains DSL Internet access from the same local telephone company (telco) that provides its wired local phone access. Thus, when DSL is used, a customer's telco is also its ISP. Each customer's DSL modem uses the existing telephone line (copper wire) to exchange data with a digital subscriber line access multiplexer (DSLAM) located in the telco's local central office (CO). The home's DSL modem takes digital data and translates it to high-frequency tones for transmission over telephone wires to the CO; the analog signals from many such houses are translated back into digital format at the DSLAM. On the customer side, a splitter separates the data and the telephone signals arriving to the home and forwards the data signal to the DSL modem. On the telco side, in the CO, the DSLAM separates the data and phone signals and sends the data into the Internet.<br />
When the downstream and upstream rates are different, the access is said to be asymmetric.<br />
**Cable Internet access** makes use of the cable television company's existing cable television infrastructure. Fiber optics connect the cable head end to neighborhood-level junctions, from which traditional coaxial cable is then used to reach individual houses and apartments. Cable internet access requires special modems, called cable modems (which is typically an external device and connects to the home PC through an Ethernet port).<br/>
One important characteristic of cable Internet access is that it is a shared broadcast medium. Every packet sent by the head end travels downstream on every link to every home and every packet sent by a home travels on the upstream channel to the head end. For this reason, if several users are simultaneously downloading a video file on the downstream channel, the actual rate at which each user receives its video file will be significantly lower than the aggregate cable downstream rate.<br/>
**Fiber To The Home (FTTH)** can potentially provide Internet access rates in the gigabits per second range.<br />

On corporate and university campuses, and increasingly in home settings, a local area network (LAN) is used to connect an end system to the edge router. Ethernet is by far the most prevalent access technology in corporate, university and home networks.










