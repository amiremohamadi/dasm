use crate::lexer::{Lexer, Sym, Token};

#[derive(Debug, PartialEq)]
pub enum Instr {
    Label(String),
    Ret,
    Nop,
}

pub struct Parser {
    index: usize,
    tokens: Vec<Token>,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Self { index: 0, tokens }
    }

    pub fn parse(&mut self) -> Vec<Instr> {
        let mut instrs = vec![];

        while let Some(token) = self.next_token() {
            match token {
                Token::Ident(label) => {
                    let next = self.next_token();
                    match next {
                        Some(Token::Symbol(Sym::Colon)) => {
                            instrs.push(Instr::Label(label.to_string()));
                        }
                        _ => {}
                    }
                }
                Token::Instruction(x) => match x.as_str() {
                    "ret" => instrs.push(Instr::Ret),
                    _ => {}
                },
                _ => {}
            }
        }

        instrs
    }

    fn next_token(&mut self) -> Option<Token> {
        if self.is_eof() {
            return None;
        }

        let token = self.tokens[self.index].clone();
        self.index += 1;
        Some(token)
    }

    fn is_eof(&self) -> bool {
        self.index >= self.tokens.len()
    }

    pub fn next_instr(&mut self) -> Instr {
        Instr::Nop
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_simple_parse() {
        let source = r#"
            section .data

            main:
            mov rax, 1
            mov bx, ax
            ret
        "#
        .to_string();
        let mut lexer = Lexer::new(source);
        let tokens = lexer.tokenize();

        let mut parser = Parser::new(tokens);
        let instrs = parser.parse();

        assert_eq!(instrs, vec![Instr::Label("main".to_string()), Instr::Ret]);
    }
}
