/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 // Reference: https://stackoverflow.com/questions/3288065/getting-gateway-to-use-for-a-given-ip-in-ansi-c

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/linux/netlink.h>
 #include <cstring>
 #include <udjat/tools/intl.h>

 #include <linux/netlink.h>
 #include <linux/rtnetlink.h>

 using namespace std;

 #define BUFFER_SIZE 16384

 UDJAT_PRIVATE bool netlink_routes(const std::function<bool(const struct nlmsghdr *msg)> &func) {

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	ssize_t received_bytes = 0;
	struct nlmsghdr *nlh = nullptr;

	{
		Socket sock{PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE};

		// 1 Sec Timeout to avoid stall
		{
			struct timeval tv;
			memset(&tv,0,sizeof(tv));
			tv.tv_sec = 1;
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
		}

		// Fill in the nlmsg header
		unsigned int msgseq = 0;
		char msgbuf[BUFFER_SIZE];
		memset(msgbuf, 0, sizeof(msgbuf));
		struct nlmsghdr *nlmsg = (struct nlmsghdr *) msgbuf;

		nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
		nlmsg->nlmsg_type = RTM_GETROUTE; 					// Get the routes from kernel routing table .
		nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;	// The message is a request for dump.
		nlmsg->nlmsg_seq = msgseq++;						// Sequence of the message packet.
		nlmsg->nlmsg_pid = getpid();						// PID of process sending the request.

		// Send
		if(send(sock, nlmsg, nlmsg->nlmsg_len, 0) < 0) {
			throw std::system_error(errno, std::system_category(), _("Cant send netlink message"));
		}

		// receive response
		char *ptr = buffer;

		int msg_len = 0;

		do {

			received_bytes = recv(sock, ptr, sizeof(buffer) - msg_len, 0);
			if (received_bytes < 0) {
				throw std::system_error(errno, std::system_category(), _("Cant receive netlink response"));
			}

			nlh = (struct nlmsghdr *) ptr;

			// Check if the header is valid
			if((NLMSG_OK(nlmsg, received_bytes) == 0) || (nlmsg->nlmsg_type == NLMSG_ERROR)) {
				throw runtime_error(_("Error in received packet"));
			}

			// If we received all data break
			if (nlh->nlmsg_type == NLMSG_DONE) {
				break;
			} else {
				ptr += received_bytes;
				msg_len += received_bytes;
			}

			// Break if its not a multi part message
			if ((nlmsg->nlmsg_flags & NLM_F_MULTI) == 0) {
				break;
			}

		} while ((nlmsg->nlmsg_seq != msgseq) || (nlmsg->nlmsg_pid != (unsigned) getpid()));
	}

	// parse response
	for ( ; NLMSG_OK(nlh, received_bytes); nlh = NLMSG_NEXT(nlh, received_bytes)) {
		if(func(nlh)) {
			return true;
		}
	}

	return false;

 }

 /*
 static int readNlSock(Socket &sock, char *bufPtr, unsigned int seqNum, unsigned int pId) {

	struct nlmsghdr *nlHdr;
	int readLen = 0, msgLen = 0;

	 do {
		// Receive response from kernel
		if ((readLen = recv(sock, bufPtr, BUFSIZE - msgLen, 0)) < 0) {
			throw system_error(errno, std::system_category(), "Unable to read NETLINK message");
		}

		nlHdr = (struct nlmsghdr *) bufPtr;

		// Check if the header is valid
		if ((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR)) {
			throw system_error(errno, std::system_category(), "Error NETLINK response");
		}

		// Check if the its the last message
		if (nlHdr->nlmsg_type == NLMSG_DONE) {
			break;
		} else {
			// move the pointer to buffer appropriately
			bufPtr += readLen;
			msgLen += readLen;
		}

		// Check if its a multi part message
		if ((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0) {
			   // Its not, return.
			break;
		}
	} while ((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));

	return msgLen;

 }

 UDJAT_PRIVATE bool netlink_routes(const std::function<bool(const struct nlmsghdr *msg)> &func) {

	struct nlmsghdr *nlMsg;
	struct rtmsg *rtMsg;
	int msgSeq = 0;
	int len = 0;
	char msgBuf[BUFSIZE];

	Socket sock{PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE};

	memset(msgBuf, 0, BUFSIZE);

	// point the header and the msg structure pointers into the buffer
	nlMsg = (struct nlmsghdr *) msgBuf;
	rtMsg = (struct rtmsg *) NLMSG_DATA(nlMsg);
	
	// Fill in the nlmsg header
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));  // Length of message.
	nlMsg->nlmsg_type = RTM_GETROUTE;   // Get the routes from kernel routing table .

	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;    // The message is a request for dump.
	nlMsg->nlmsg_seq = msgSeq++;    // Sequence of the message packet.
	nlMsg->nlmsg_pid = getpid();    // PID of process sending the request.
	
	// Send the request
	if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0) {
		throw system_error(errno, std::system_category(), "Unable to send NETLINK message");
	}
	
	// Read the response
	len = readNlSock(sock, msgBuf, msgSeq, getpid());

	// Loop through the response.
	for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) {

		if(func(nlMsg)) {
			return true;
		}

		struct nlmsghdr *nlHdr = nlMsg; // TODO: Remove it and use nlMsg directly.

		// Ignore if the route is not for AF_INET or does not belong to main routing table.
		if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)) {
			continue;
		}

		struct rtattr *rtAttr = (struct rtattr *) RTM_RTA(rtMsg);
		int rtAttrLen = RTM_PAYLOAD(nlHdr);
		for (; RTA_OK(rtAttr, rtAttrLen); rtAttr = RTA_NEXT(rtAttr, rtAttrLen)) {
			if (func(rtAttr,(void *) nlMsg) {
				return true;
			}
		}

	}

	return false;

 }
 */
