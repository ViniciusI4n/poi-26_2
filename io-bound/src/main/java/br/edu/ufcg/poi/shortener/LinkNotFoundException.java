package br.edu.ufcg.poi.shortener;

public class LinkNotFoundException extends RuntimeException {

	public LinkNotFoundException(String code) {
		super("Link '%s' não encontrado".formatted(code));
	}

}
