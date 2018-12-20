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
import com.meituan.dorado.codec.octo.meta.Header;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HeartbeatHandler extends SimpleChannelInboundHandler<Header> {
    private static final Logger log = LoggerFactory.getLogger(HeartbeatHandler.class);
    private final SettableFuture<Integer> result;

    public HeartbeatHandler(SettableFuture<Integer> result) {
        this.result = result;
    }

    @Override
    protected void channelRead0(ChannelHandlerContext channelHandlerContext, Header header) throws Exception {
        result.set(header.getHeartbeatInfo().getStatus());
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        log.error("", cause);
        ctx.close();
    }
}
