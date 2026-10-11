package br.edu.ufcg.poi.pow.dto;

public record PowResponse(
		String data,
		int difficulty,
		long nonce,
		String hash,
		long attempts,
		long elapsedMs) {
}
