pub struct Lexer {
    index: usize,
    pub buf: Vec<char>,
}

#[derive(Debug, PartialEq)]
pub enum Sym {
    Comment,
    Comma,
    Colon,
    LBrace,
    RBrace,
}
#[derive(Debug, PartialEq)]
pub enum Token {
    Int(i32),
    Symbol(Sym),
    Ident(String),
    Dummy,
}

impl Lexer {
    pub fn new(buf: String) -> Self {
        Self {
            index: 0,
            buf: buf.chars().collect(),
        }
    }

    pub fn tokenize(&mut self) -> Vec<Token> {
        let mut tokens = vec![];

        while let Some(token) = self.next_token() {
            tokens.push(token);
        }

        tokens
    }

    pub fn next_token(&mut self) -> Option<Token> {
        self.consume_whitespace();

        if self.is_eof() {
            return None;
        }

        match self.peek_char() {
            x if x.is_digit(10) => Some(self.consume_number()),
            x if is_ident(x) => Some(self.consume_ident()),
            _ => Some(self.consume_symbol()),
        }
    }

    fn is_eof(&self) -> bool {
        self.index >= self.buf.len()
    }

    fn consume_whitespace(&mut self) {
        while !self.is_eof() && self.peek_char().is_whitespace() {
            self.consume_char();
        }
    }

    fn consume_number(&mut self) -> Token {
        let mut num = String::new();

        while !self.is_eof() && self.peek_char().is_digit(10) {
            num.push(self.consume_char());
        }

        Token::Int(num.parse().unwrap())
    }

    fn consume_char(&mut self) -> char {
        let c = self.buf[self.index];
        self.index += 1;
        c
    }

    fn consume_symbol(&mut self) -> Token {
        let s = match self.consume_char() {
            ',' => Sym::Comma,
            ':' => Sym::Colon,
            _ => Sym::Comment,
        };

        Token::Symbol(s)
    }

    fn consume_ident(&mut self) -> Token {
        let mut name = String::new();

        while !self.is_eof()
            && (self.peek_char().is_alphanumeric()
                || self.peek_char() == '.'
                || self.peek_char() == '_')
        {
            name.push(self.consume_char());
        }

        Token::Ident(name)
    }

    fn peek_char(&self) -> char {
        self.buf[self.index]
    }
}

fn is_ident(c: char) -> bool {
    c.is_alphabetic() || c == '.' || c == '_'
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_simple_tokenize() {
        let source = r#"
            section .data

            main:
            mov rax, 1
            mov bx, ax
        "#
        .to_string();
        let mut lexer = Lexer::new(source);
        let tokens = lexer.tokenize();

        assert_eq!(tokens.len(), 12);
        assert_eq!(
            tokens,
            vec![
                Token::Ident("section".to_string()),
                Token::Ident(".data".to_string()),
                Token::Ident("main".to_string()),
                Token::Symbol(Sym::Colon),
                Token::Ident("mov".to_string()),
                Token::Ident("rax".to_string()),
                Token::Symbol(Sym::Comma),
                Token::Int(1),
                Token::Ident("mov".to_string()),
                Token::Ident("bx".to_string()),
                Token::Symbol(Sym::Comma),
                Token::Ident("ax".to_string())
            ]
        );
    }
}
