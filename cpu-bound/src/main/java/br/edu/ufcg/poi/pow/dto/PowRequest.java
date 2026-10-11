package br.edu.ufcg.poi.pow.dto;

import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;

public record PowRequest(
		@NotBlank @Size(max = 1024) String data,
		@Min(1) int difficulty) {
}
