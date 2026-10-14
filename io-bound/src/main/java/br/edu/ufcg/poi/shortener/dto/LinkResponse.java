package br.edu.ufcg.poi.shortener.dto;

import java.time.Instant;

public record LinkResponse(
		String code,
		String url,
		String shortUrl,
		long clicks,
		Instant createdAt) {
}
