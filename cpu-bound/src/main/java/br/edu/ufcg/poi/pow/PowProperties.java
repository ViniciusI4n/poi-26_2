package br.edu.ufcg.poi.pow;

import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * Limites do serviço, configuráveis via application.yml ou variáveis de ambiente.
 *
 * @param maxDifficulty dificuldade máxima aceita
 * @param timeoutMs     tempo máximo de busca por requisição
 */
@ConfigurationProperties(prefix = "pow")
public record PowProperties(int maxDifficulty, long timeoutMs) {
}
