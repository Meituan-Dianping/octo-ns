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

package com.meituan.octo.mns.scanner.util;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Constant {
    public static final int timeout;
    public static final int STATUS_ALIVE = 2;
    public static final int STATUS_DEAD = 0;
    public static final int STATUS_STOPPED = 4;
    public static final int HEARTBEAT_SUPPORT_SCANNER = 2;
    public static final int HEARTBEAT_SUPPORT_BOTH = 3;
    private static final Logger log = LoggerFactory.getLogger(Constant.class);
    private static final int DEFAULT_TIMEOUT = 1000;

    static {
        int finalTimeout;
        String timeoutValue = System.getProperty("provider.detect.timeout");
        if (timeoutValue != null) {
            try {
                finalTimeout = Integer.parseInt(timeoutValue);
                log.info("set timeout to {}", finalTimeout);
            } catch (Exception e) {
                log.warn("parse provider.detect.timeout exception, will use default value {}", DEFAULT_TIMEOUT);
                finalTimeout = DEFAULT_TIMEOUT;
            }
        } else {
            log.info("provider.detect.timeout is not provided, use default value {}", DEFAULT_TIMEOUT);
            finalTimeout = DEFAULT_TIMEOUT;
        }
        timeout = finalTimeout;
    }
}
