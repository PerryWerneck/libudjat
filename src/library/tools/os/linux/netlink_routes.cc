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

 #include <linux/netlink.h>
 #include <linux/rtnetlink.h>

 using namespace std;

 #define BUFSIZE 16384

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

 UDJAT_PRIVATE bool netlink_routes(const std::function<bool(const struct rtattr *rtAttr)> &func) {

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

		struct nlmsghdr *nlHdr = nlMsg; // TODO: Remove it and use nlMsg directly.

		// Ignore if the route is not for AF_INET or does not belong to main routing table. */
		if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)) {
			continue;
		}

		struct rtattr *rtAttr = (struct rtattr *) RTM_RTA(rtMsg);
		int rtAttrLen = RTM_PAYLOAD(nlHdr);
		for (; RTA_OK(rtAttr, rtAttrLen); rtAttr = RTA_NEXT(rtAttr, rtAttrLen)) {
			if (func(rtAttr)) {
				return true;
			}
		}

	}

	return false;

 }
