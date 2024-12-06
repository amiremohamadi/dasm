use crate::error::Error;
use crate::lexer::{Sym, Token};

#[derive(Debug, PartialEq)]
pub enum Operand {
    Register(String),
    Label(String),
    Integer(i32),
}

#[derive(Debug, PartialEq)]
pub enum Instr {
    Label(String),
    Mov(Operand, Operand),
    Add(Operand, Operand),
    Jmp(Operand),
    Syscall,
    Ret,
    Nop,
}

impl Instr {
    pub fn encode(&self) -> Vec<u8> {
        match self {
            Self::Mov(Operand::Register(r), Operand::Integer(x)) => {
                let opcode = 0xb8
                    + match r.as_str() {
                        "rcx" => 1,
                        "rdx" => 2,
                        "rbx" => 3,
                        "rsp" => 4,
                        "rbp" => 5,
                        "rsi" => 6,
                        "rdi" => 7,
                        _ => 0,
                    };
                let mut code = vec![opcode];
                code.extend_from_slice(&x.to_le_bytes());
                code
            }
            Self::Syscall => {
                vec![0x0f, 0x05]
            }
            _ => vec![],
        }
    }
}

pub struct Parser {
    index: usize,
    tokens: Vec<Token>,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Self { index: 0, tokens }
    }

    pub fn parse(&mut self) -> Result<Vec<Instr>, Error> {
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
                    "syscall" => instrs.push(Instr::Syscall),
                    "jmp" => {
                        let op = self.parse_unary_op()?;
                        instrs.push(Instr::Jmp(op));
                    }
                    "mov" => {
                        let (op1, op2) = self.parse_binary_op()?;
                        instrs.push(Instr::Mov(op1, op2));
                    }
                    "add" => {
                        let (op1, op2) = self.parse_binary_op()?;
                        instrs.push(Instr::Add(op1, op2));
                    }
                    _ => return Err(Error(format!("invalid instruction {}", x))),
                },
                _ => return Err(Error(format!("invalid token {:?}", token))),
            }
        }

        Ok(instrs)
    }

    fn parse_unary_op(&mut self) -> Result<Operand, Error> {
        match self.next_token() {
            Some(Token::Register(r)) => Ok(Operand::Register(r)),
            Some(Token::Int(x)) => Ok(Operand::Integer(x)),
            Some(Token::Ident(x)) => Ok(Operand::Label(x)),
            _ => Err(Error("failed to parse unary op".to_string())),
        }
    }

    fn parse_binary_op(&mut self) -> Result<(Operand, Operand), Error> {
        match self.next_token() {
            Some(Token::Register(r1)) => {
                if let Some(Token::Symbol(Sym::Comma)) = self.next_token() {
                    let op1 = Operand::Register(r1);
                    match self.next_token() {
                        Some(Token::Register(r2)) => {
                            let op2 = Operand::Register(r2);
                            return Ok((op1, op2));
                        }
                        Some(Token::Int(x)) => {
                            let op2 = Operand::Integer(x);
                            return Ok((op1, op2));
                        }
                        _ => {}
                    }
                }
            }
            _ => {}
        }

        Err(Error("failed to parse binary op".to_string()))
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
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::lexer::Lexer;

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
        let instrs = parser.parse().unwrap();

        assert_eq!(
            instrs,
            vec![
                Instr::Label("main".to_string()),
                Instr::Mov(Operand::Register("rax".to_string()), Operand::Integer(1),),
                Instr::Mov(
                    Operand::Register("bx".to_string()),
                    Operand::Register("ax".to_string())
                ),
                Instr::Ret
            ]
        );
    }
}
