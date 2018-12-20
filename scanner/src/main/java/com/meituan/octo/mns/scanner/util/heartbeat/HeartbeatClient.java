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

import com.google.common.util.concurrent.SettableFuture;
import com.meituan.dorado.codec.octo.meta.CallType;
import com.meituan.dorado.codec.octo.meta.Header;
import com.meituan.dorado.codec.octo.meta.MessageType;
import com.meituan.dorado.codec.octo.meta.RequestInfo;
import com.meituan.octo.mns.scanner.util.Constant;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class HeartbeatClient {
    private static final Logger log = LoggerFactory.getLogger(HeartbeatClient.class);
    private static final Header header;
    private static EventLoopGroup heartbeatGroup = new NioEventLoopGroup();

    static {
        header = new Header();
        header.setMessageType((byte) MessageType.ScannerHeartbeat.getValue());

        RequestInfo requestInfo = new RequestInfo();
        requestInfo.setSequenceId(1);
        requestInfo.setTimeout(Constant.timeout);
        requestInfo.setServiceName("ScannerHeartbeat");
        requestInfo.setCallType((byte) CallType.Reply.getValue());
        header.setRequestInfo(requestInfo);
        header.setMessageType(((byte) MessageType.ScannerHeartbeat.getValue()));
    }

    public static int detect(String ip, int port) {
        SettableFuture<Integer> result = SettableFuture.create();
        Bootstrap bootstrap = new Bootstrap();
        bootstrap.group(heartbeatGroup).option(ChannelOption.TCP_NODELAY, true).option(ChannelOption.SO_REUSEADDR, true)
                .channel(NioSocketChannel.class).handler(new ChannelInitializer<SocketChannel>() {
            @Override
            protected void initChannel(SocketChannel socketChannel) throws Exception {
                socketChannel.pipeline().addLast(new HeartbeatCodec()).addLast(new HeartbeatHandler(result));
            }
        });
        Channel channel = null;
        try {
            ChannelFuture future = bootstrap.connect(ip, port);
            future.get(Constant.timeout, TimeUnit.MILLISECONDS);
            channel = future.channel();
            if (channel.isActive() && channel.isWritable()) {
                channel.writeAndFlush(header);
                try {
                    return result.get(Constant.timeout, TimeUnit.MILLISECONDS);
                } catch (TimeoutException e) {
                    log.warn("detect:{}:{} timeout, timeout:{}", ip, port, Constant.timeout);
                    return Constant.STATUS_DEAD;
                } catch (Exception e) {
                    log.error("", e);
                    return Constant.STATUS_DEAD;
                }
            } else {
                return Constant.STATUS_DEAD;
            }
        } catch (TimeoutException e) {
            log.warn("detect:{}:{} timeout, timeout:{}", ip, port, Constant.timeout);
            return Constant.STATUS_DEAD;
        } catch (ExecutionException e) {
            log.warn("detect:{}:{} exception, exception:{}, message:{}", ip, port, e.getClass(), e.getMessage());
            return Constant.STATUS_DEAD;
        } catch (Exception e) {
            log.error("", e);
            return Constant.STATUS_DEAD;
        } finally {
            if (channel != null) {
                channel.disconnect();
            }
        }

    }
}
