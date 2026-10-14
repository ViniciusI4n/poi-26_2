package br.edu.ufcg.poi.shortener;

import java.time.Instant;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

@Entity
@Table(name = "links")
public class Link {

	@Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	private Long id;

	@Column(nullable = false, unique = true, length = 16)
	private String code;

	@Column(nullable = false, length = 2048)
	private String url;

	@Column(nullable = false)
	private long clicks;

	@Column(name = "created_at", nullable = false)
	private Instant createdAt;

	protected Link() {
	}

	public Link(String code, String url) {
		this.code = code;
		this.url = url;
		this.createdAt = Instant.now();
	}

	public Long getId() {
		return id;
	}

	public String getCode() {
		return code;
	}

	public String getUrl() {
		return url;
	}

	public long getClicks() {
		return clicks;
	}

	public Instant getCreatedAt() {
		return createdAt;
	}

}
