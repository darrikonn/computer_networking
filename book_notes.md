# Book Notes

## What is the Internet?
The Internet is a computer network that interconnects hundreds of millions of computing devices throughout the world. All of these devices are called **hosts** or **end systems**. <br />
End systems aare connected together by a network of **communication links** and **packet switches**. There are many types of communication links, which are made up of different types of physical media, including coaxial cable, copper wire, optical fiber and radio spectrum. Different links can transmit data at different rates, with the **transmission rate** of a link measured in bits/second. When one end system has data to send to another end system, the sending end system segments the data and adds header bytes to each segment. The resulting packages of information, known as **packets** in the jargon of computer networks, are then sent through the network to the destination end system, where they are reassembled into the original data. <br />
End systems access the Internet through **Internet Service Providers (ISPs)**, including local cable or telephone companies, corporate ISPs, university ISPs etc. Each ISP is in itself a network of packet switches and communication links. ISPs provide variety of types of network access to the end systems, including residential broadband access such as cable modem or DSL. <br />
The Internet is all about connecting end systems to each other, so the ISPs that provide access to end systems must also be interconnected. These lower-tier ISPs are interconnected through national and international upper-tier ISPs such as Level 3 Communication, AT&T etc. An upper-tier ISP consists of high-speed routers interconnected with high-speed fiber-optic links. Each ISP network is managed independently, runs the IP protocol and conforms to certain naming and address conventions. <br />
End systems, packet switches, and other pieces of the Internet run **protocols** that control the sending and receiving of information within the Internet. The **Transmission Control Protocol (TCP)** and the **Internet Protocol (IP)** are two of the most important protocols in the Internet. <br />
The IP protocol specifies the format of the packets that are sent and received among routers and end systems. The Internet's principal protocols are collectively known as **TCP/IP**. <br />
**Internet standards** are developed by Internet Engineering Task Force (IETF). The IETF standards documents are called **requests for comments (RCFs)**. They define protocols such as TCP, IP, HTTP and SMTP.

### A Services Description
**Distributed application** involve multiple end systems that exchange data with each other. Internet applications run on end systems - they do not run in the packet switches in the network core. <br />
End systems attached to the Internet provide an **Application Programming Interface (API)** that specifies how a program running on one end system asks the Internet infrastructure to deliver data to a specific destination program running on another end system. This Internet API is a set of rules that the sending program must follow so that the Internet can deliver the data to the destination program.

### What Is a Protocol


