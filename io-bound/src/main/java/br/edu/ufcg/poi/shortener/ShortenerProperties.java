package br.edu.ufcg.poi.shortener;

import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * Configurações do encurtador, definidas via application.yml ou variáveis de ambiente.
 *
 * @param baseUrl                URL pública usada para montar o link curto
 * @param simulatedDbLatencyMs   se maior que 0, executa {@code pg_sleep} antes de cada operação,
 *                               simulando um banco remoto/lento
 */
@ConfigurationProperties(prefix = "shortener")
public record ShortenerProperties(String baseUrl, long simulatedDbLatencyMs) {
}
