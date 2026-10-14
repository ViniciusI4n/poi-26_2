package br.edu.ufcg.poi.shortener;

import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Modifying;
import org.springframework.data.jpa.repository.Query;

public interface LinkRepository extends JpaRepository<Link, Long> {

	Optional<Link> findByCode(String code);

	@Query("select l.url from Link l where l.code = :code")
	Optional<String> findUrlByCode(String code);

	@Modifying
	@Query("update Link l set l.clicks = l.clicks + 1 where l.code = :code")
	int incrementClicks(String code);

}
