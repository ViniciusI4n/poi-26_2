package br.edu.ufcg.poi.pow;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.ConfigurationPropertiesScan;

@SpringBootApplication
@ConfigurationPropertiesScan
public class PowApplication {

	public static void main(String[] args) {
		SpringApplication.run(PowApplication.class, args);
	}

}
