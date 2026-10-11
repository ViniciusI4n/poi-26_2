package br.edu.ufcg.poi.pow.dto;

public record VerifyResponse(
		String data,
		int difficulty,
		long nonce,
		String hash,
		boolean valid) {
}
