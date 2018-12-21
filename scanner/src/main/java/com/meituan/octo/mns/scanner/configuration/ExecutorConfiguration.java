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

package com.meituan.octo.mns.scanner.configuration;

import com.meituan.dorado.common.thread.DefaultThreadFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

@Configuration
public class ExecutorConfiguration {
    @Bean("aliveCheckExecutor")
    public Executor aliveCheckExecutor(@Value("${executor.alive.threadCount}") int threadCount) {
        return Executors.newFixedThreadPool(threadCount, new DefaultThreadFactory("aliveCheckExecutor"));
    }

    @Bean("deadCheckExecutor")
    public Executor deadCheckExecutor(@Value("${executor.dead.threadCount}") int threadCount) {
        return Executors.newFixedThreadPool(threadCount, new DefaultThreadFactory("deadCheckExecutor"));
    }
}
