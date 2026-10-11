package br.edu.ufcg.poi.shortener.dto;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;

public record CreateLinkRequest(
		@NotBlank @Size(max = 2048) @Pattern(regexp = "^https?://\\S+$", message = "deve ser uma URL http(s)") String url) {
}
