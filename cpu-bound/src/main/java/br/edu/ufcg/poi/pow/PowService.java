package br.edu.ufcg.poi.pow;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HexFormat;

import br.edu.ufcg.poi.pow.dto.PowResponse;
import br.edu.ufcg.poi.pow.dto.VerifyResponse;
import io.micrometer.core.instrument.Counter;
import io.micrometer.core.instrument.MeterRegistry;
import io.micrometer.core.instrument.Timer;
import org.springframework.stereotype.Service;

@Service
public class PowService {

	private static final long CHECK_INTERVAL = 100_000;

	private static final ThreadLocal<MessageDigest> SHA256 = ThreadLocal.withInitial(() -> {
		try {
			return MessageDigest.getInstance("SHA-256");
		}
		catch (NoSuchAlgorithmException e) {
			throw new IllegalStateException(e);
		}
	});

	private final PowProperties properties;
	private final MeterRegistry registry;
	private final Counter attemptsCounter;

	public PowService(PowProperties properties, MeterRegistry registry) {
		this.properties = properties;
		this.registry = registry;
		this.attemptsCounter = Counter.builder("pow.attempts")
				.description("Total de hashes SHA-256 calculados na busca por nonces")
				.register(registry);
	}

	public PowResponse solve(String data, int difficulty) {
		Timer.Sample sample = Timer.start(registry);
		long start = System.nanoTime();
		long deadline = start + properties.timeoutMs() * 1_000_000;
		byte[] prefix = data.getBytes(StandardCharsets.UTF_8);
		MessageDigest digest = SHA256.get();

		String outcome = "timeout";
		long nonce = 0;
		try {
			while (true) {
				byte[] hash = hash(digest, prefix, nonce);
				if (hasLeadingZeroNibbles(hash, difficulty)) {
					outcome = "solved";
					long elapsedMs = (System.nanoTime() - start) / 1_000_000;
					return new PowResponse(data, difficulty, nonce, HexFormat.of().formatHex(hash), nonce + 1,
							elapsedMs);
				}
				nonce++;
				if (nonce % CHECK_INTERVAL == 0
						&& (System.nanoTime() > deadline || Thread.currentThread().isInterrupted())) {
					throw new PowTimeoutException(difficulty, nonce, properties.timeoutMs());
				}
			}
		}
		finally {
			attemptsCounter.increment(outcome.equals("solved") ? nonce + 1 : nonce);
			sample.stop(Timer.builder("pow.solve")
					.description("Tempo para encontrar um nonce válido")
					.tag("difficulty", String.valueOf(difficulty))
					.tag("outcome", outcome)
					.publishPercentileHistogram()
					.register(registry));
		}
	}

	public VerifyResponse verify(String data, long nonce, int difficulty) {
		byte[] hash = hash(SHA256.get(), data.getBytes(StandardCharsets.UTF_8), nonce);
		return new VerifyResponse(data, difficulty, nonce, HexFormat.of().formatHex(hash),
				hasLeadingZeroNibbles(hash, difficulty));
	}

	private static byte[] hash(MessageDigest digest, byte[] prefix, long nonce) {
		digest.update(prefix);
		digest.update(Long.toString(nonce).getBytes(StandardCharsets.US_ASCII));
		return digest.digest();
	}

	static boolean hasLeadingZeroNibbles(byte[] hash, int n) {
		int fullBytes = n / 2;
		for (int i = 0; i < fullBytes; i++) {
			if (hash[i] != 0) {
				return false;
			}
		}
		return n % 2 == 0 || (hash[fullBytes] & 0xF0) == 0;
	}

}
