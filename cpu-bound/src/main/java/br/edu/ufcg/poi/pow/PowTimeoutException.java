package br.edu.ufcg.poi.pow;

public class PowTimeoutException extends RuntimeException {

	public PowTimeoutException(int difficulty, long attempts, long timeoutMs) {
		super("Nonce não encontrado para dificuldade %d após %d tentativas (timeout de %d ms)"
				.formatted(difficulty, attempts, timeoutMs));
	}

}
