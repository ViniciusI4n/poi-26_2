package br.edu.ufcg.poi.shortener;

import java.security.SecureRandom;
import java.util.List;

import br.edu.ufcg.poi.shortener.dto.LinkResponse;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Sort;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.transaction.support.TransactionTemplate;

@Service
public class LinkService {

	static final int CODE_LENGTH = 7;
	private static final String ALPHABET = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	private static final int MAX_CREATE_ATTEMPTS = 3;

	private final SecureRandom random = new SecureRandom();
	private final LinkRepository repository;
	private final JdbcTemplate jdbc;
	private final TransactionTemplate tx;
	private final ShortenerProperties properties;

	public LinkService(LinkRepository repository, JdbcTemplate jdbc, TransactionTemplate tx,
			ShortenerProperties properties) {
		this.repository = repository;
		this.jdbc = jdbc;
		this.tx = tx;
		this.properties = properties;
	}

	public LinkResponse create(String url) {
		for (int attempt = 1;; attempt++) {
			try {
				Link link = tx.execute(status -> {
					simulateDbLatency();
					return repository.saveAndFlush(new Link(randomCode(), url));
				});
				return toResponse(link);
			}
			catch (DataIntegrityViolationException e) {
				if (attempt >= MAX_CREATE_ATTEMPTS) {
					throw e;
				}
			}
		}
	}

	@Transactional
	public String resolve(String code) {
		simulateDbLatency();
		if (repository.incrementClicks(code) == 0) {
			throw new LinkNotFoundException(code);
		}
		return repository.findUrlByCode(code).orElseThrow(() -> new LinkNotFoundException(code));
	}

	@Transactional(readOnly = true)
	public LinkResponse get(String code) {
		simulateDbLatency();
		return repository.findByCode(code).map(this::toResponse).orElseThrow(() -> new LinkNotFoundException(code));
	}

	@Transactional(readOnly = true)
	public List<LinkResponse> list(int page, int size) {
		simulateDbLatency();
		return repository.findAll(PageRequest.of(page, size, Sort.by(Sort.Direction.DESC, "createdAt")))
			.map(this::toResponse)
			.getContent();
	}

	private void simulateDbLatency() {
		if (properties.simulatedDbLatencyMs() > 0) {
			jdbc.execute("SELECT pg_sleep(" + properties.simulatedDbLatencyMs() / 1000.0 + ")");
		}
	}

	String randomCode() {
		StringBuilder code = new StringBuilder(CODE_LENGTH);
		for (int i = 0; i < CODE_LENGTH; i++) {
			code.append(ALPHABET.charAt(random.nextInt(ALPHABET.length())));
		}
		return code.toString();
	}

	private LinkResponse toResponse(Link link) {
		return new LinkResponse(link.getCode(), link.getUrl(), properties.baseUrl() + "/" + link.getCode(),
				link.getClicks(), link.getCreatedAt());
	}

}
