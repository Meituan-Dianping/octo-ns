package com.meituan.octo.mnsc;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.ImportResource;

@SpringBootApplication
@ImportResource(locations = {"classpath:applicationContext.xml", "classpath:webmvc-config.xml"})
public class MnscServerApplication {
    public static void main(String[] args) {
        SpringApplication.run(MnscServerApplication.class, args);
    }
}
