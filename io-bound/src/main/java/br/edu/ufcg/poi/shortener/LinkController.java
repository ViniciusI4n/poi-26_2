package br.edu.ufcg.poi.shortener;

import java.net.URI;
import java.util.List;

import br.edu.ufcg.poi.shortener.dto.CreateLinkRequest;
import br.edu.ufcg.poi.shortener.dto.LinkResponse;
import jakarta.validation.Valid;
import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.Min;
import org.springframework.http.HttpStatus;
import org.springframework.http.ProblemDetail;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class LinkController {

	private final LinkService service;

	public LinkController(LinkService service) {
		this.service = service;
	}

	@PostMapping("/api/links")
	public ResponseEntity<LinkResponse> create(@Valid @RequestBody CreateLinkRequest request) {
		LinkResponse link = service.create(request.url());
		return ResponseEntity.created(URI.create("/api/links/" + link.code())).body(link);
	}

	@GetMapping("/api/links/{code}")
	public LinkResponse get(@PathVariable String code) {
		return service.get(code);
	}

	@GetMapping("/api/links")
	public List<LinkResponse> list(@RequestParam(defaultValue = "0") @Min(0) int page,
			@RequestParam(defaultValue = "20") @Min(1) @Max(100) int size) {
		return service.list(page, size);
	}

	@GetMapping("/{code:[0-9a-zA-Z]{" + LinkService.CODE_LENGTH + "}}")
	public ResponseEntity<Void> redirect(@PathVariable String code) {
		return ResponseEntity.status(HttpStatus.FOUND).location(URI.create(service.resolve(code))).build();
	}

	@ExceptionHandler(LinkNotFoundException.class)
	ProblemDetail handleNotFound(LinkNotFoundException e) {
		return ProblemDetail.forStatusAndDetail(HttpStatus.NOT_FOUND, e.getMessage());
	}

}
