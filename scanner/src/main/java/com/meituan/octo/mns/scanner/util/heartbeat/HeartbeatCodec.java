/*
 * Copyright 2018 Meituan Dianping. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.meituan.octo.mns.scanner.util.heartbeat;

import com.meituan.dorado.codec.octo.DoradoCodec;
import com.meituan.dorado.codec.octo.meta.Header;
import com.meituan.dorado.common.exception.ProtocolException;
import com.meituan.dorado.transport.meta.DefaultRequest;
import com.meituan.dorado.util.BytesUtil;
import com.meituan.dorado.util.CompressUtil;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageCodec;
import org.apache.thrift.TDeserializer;
import org.apache.thrift.TException;
import org.apache.thrift.TSerializer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

public class HeartbeatCodec extends ByteToMessageCodec<Header> {
    private static final Logger log = LoggerFactory.getLogger(HeartbeatCodec.class);
    private static final DoradoCodec codec = new DoradoCodec();

    @Override
    protected void encode(ChannelHandlerContext channelHandlerContext, Header header, ByteBuf byteBuf)
            throws Exception {
        byte[] headerBytes;
        try {
            headerBytes = new TSerializer().serialize(header);
        } catch (TException e) {
            log.error("", e);
            return;
        }

        DefaultRequest request = new DefaultRequest();
        request.setDoChecksum(false);
        request.setCompressType(CompressUtil.CompressType.NO);
        request.setVersion((byte) 0x00);
        request.setSerialize((byte) 0x01);

        byte[] total = codec.generateSendMessageBuff(request, headerBytes, new byte[0]);
        byteBuf.writeBytes(total);
    }

    @Override
    protected void decode(ChannelHandlerContext channelHandlerContext, ByteBuf byteBuf, List<Object> list)
            throws Exception {
        if (byteBuf.readableBytes() < 10) {
            return;
        }
        byte[] buffer = new byte[byteBuf.readableBytes()];
        byteBuf.markReaderIndex();
        byteBuf.readBytes(buffer);
        if (!isXMDProtocol(buffer)) {
            throw new RuntimeException();
        } else {
            byte[] headerBodyBytes = this.getHeaderBodyBuff(buffer);
            int headerBodyLength = headerBodyBytes.length;
            short headerLength = BytesUtil.bytes2short(buffer, 8);
            byte[] headerBytes = new byte[headerLength];
            System.arraycopy(headerBodyBytes, 0, headerBytes, 0, headerLength);
            byte[] bodyBytes = new byte[headerBodyLength - headerLength];
            System.arraycopy(headerBodyBytes, headerLength, bodyBytes, 0, headerBodyLength - headerLength);

            Header header = null;

            try {
                header = decodeXMDHeader(headerBytes);
                list.add(header);
            } catch (TException var13) {
                throw new ProtocolException("Deserialize XMD header failed.", var13);
            }
        }

    }

    private Header decodeXMDHeader(byte[] headerBytes) throws TException {
        TDeserializer deserializer = new TDeserializer();
        Header header = new Header();
        deserializer.deserialize(header, headerBytes);
        return header;
    }

    private boolean isXMDProtocol(byte[] buffer) {
        short magic = BytesUtil.bytes2short(buffer, 0);
        return magic == -21574;
    }

    private byte[] getHeaderBodyBuff(byte[] buffer) {
        int readerIndex = 4;
        int totalLength = BytesUtil.bytes2int(buffer, readerIndex);
        readerIndex += 6;

        int msgNeedLength = totalLength + 8;
        if (buffer.length < msgNeedLength) {
            throw new RuntimeException("Message length less than need length");
        }

        int headerBodyLength = totalLength - 2;

        byte[] headerBodyBytes = new byte[headerBodyLength];
        System.arraycopy(buffer, readerIndex, headerBodyBytes, 0, headerBodyLength);
        return headerBodyBytes;
    }
}
