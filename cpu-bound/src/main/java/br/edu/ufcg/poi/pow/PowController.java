package br.edu.ufcg.poi.pow;

import br.edu.ufcg.poi.pow.dto.PowRequest;
import br.edu.ufcg.poi.pow.dto.PowResponse;
import br.edu.ufcg.poi.pow.dto.VerifyResponse;
import jakarta.validation.Valid;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotBlank;
import org.springframework.http.HttpStatus;
import org.springframework.http.ProblemDetail;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/pow")
public class PowController {

	private final PowService service;
	private final PowProperties properties;

	public PowController(PowService service, PowProperties properties) {
		this.service = service;
		this.properties = properties;
	}

	@PostMapping
	public PowResponse solve(@Valid @RequestBody PowRequest request) {
		checkMaxDifficulty(request.difficulty());
		return service.solve(request.data(), request.difficulty());
	}

	@GetMapping("/verify")
	public VerifyResponse verify(@RequestParam @NotBlank String data, @RequestParam long nonce,
			@RequestParam @Min(1) int difficulty) {
		checkMaxDifficulty(difficulty);
		return service.verify(data, nonce, difficulty);
	}

	private void checkMaxDifficulty(int difficulty) {
		if (difficulty > properties.maxDifficulty()) {
			throw new IllegalArgumentException(
					"difficulty deve estar entre 1 e " + properties.maxDifficulty());
		}
	}

	@ExceptionHandler(IllegalArgumentException.class)
	ProblemDetail handleBadRequest(IllegalArgumentException e) {
		return ProblemDetail.forStatusAndDetail(HttpStatus.BAD_REQUEST, e.getMessage());
	}

	@ExceptionHandler(PowTimeoutException.class)
	ProblemDetail handleTimeout(PowTimeoutException e) {
		return ProblemDetail.forStatusAndDetail(HttpStatus.SERVICE_UNAVAILABLE, e.getMessage());
	}

}
