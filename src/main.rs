mod error;
mod lexer;
mod parser;

use std::env;
use std::fs;

use error::Error;
use lexer::Lexer;
use parser::Parser;

fn main() -> Result<(), Error> {
    let mut tokens = Vec::new();
    let args = env::args().collect::<Vec<String>>();

    if args.len() < 3 {
        println!("usage: ./dasm filename.s <mode> (<mode>: --lex, --parse)");
        std::process::exit(1);
    }

    let filename = &args[1];
    let mode = &args[2];

    if let Ok(buf) = fs::read(filename) {
        let buf = String::from_utf8_lossy(&buf);
        let mut lexer = Lexer::new(buf.into_owned());

        tokens = lexer.tokenize();
    }

    match mode.as_str() {
        "--lex" => {
            for token in tokens {
                println!("{:?}", token);
            }
        }
        "--parse" => {
            let mut parser = Parser::new(tokens);
            let insts = parser.parse()?;
            for inst in insts {
                println!("{:?}", inst);
            }
        }
        _ => {}
    }

    Ok(())
}
