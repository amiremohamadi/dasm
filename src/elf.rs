use std::mem::size_of;

// https://gist.github.com/x0nu11byt3/bcb35c3de461e5fb66173071a2379779
#[derive(Default, Debug)]
pub struct Header {
    pub ident: u128,
    pub r#type: u16,
    pub machine: u16,
    pub version: u32,
    pub entry: u64,
    pub program_header_offset: u64,
    pub section_header_offset: u64,
    pub flags: u32,
    pub header_size: u16,
    pub program_header_size: u16,
    pub program_header_num: u16,
    pub section_header_size: u16,
    pub section_header_num: u16,
    pub str_index: u16,
}

impl Header {
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut buf = Vec::<u8>::new();
        buf.extend(self.ident.to_be_bytes());
        buf.extend(self.r#type.to_le_bytes());
        buf.extend(self.machine.to_le_bytes());
        buf.extend(self.version.to_le_bytes());
        buf.extend(self.entry.to_le_bytes());
        buf.extend(self.program_header_offset.to_le_bytes());
        buf.extend(self.section_header_offset.to_le_bytes());
        buf.extend(self.flags.to_le_bytes());
        buf.extend(self.header_size.to_le_bytes());
        buf.extend(self.program_header_size.to_le_bytes());
        buf.extend(self.program_header_num.to_le_bytes());
        buf.extend(self.section_header_size.to_le_bytes());
        buf.extend(self.section_header_num.to_le_bytes());
        buf.extend(self.str_index.to_le_bytes());
        buf
    }
}

#[derive(Default, Debug)]
pub struct SectionHeader {
    pub name: u32,
    pub r#type: u32,
    pub flags: u64,
    pub address: u64,
    pub offset: u64,
    pub size: u64,
    pub link: u32,
    pub info: u32,
    pub address_align: u64,
    pub entry_size: u64,
}

impl SectionHeader {
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut buf = Vec::<u8>::new();
        buf.extend(self.name.to_le_bytes());
        buf.extend(self.r#type.to_le_bytes());
        buf.extend(self.flags.to_le_bytes());
        buf.extend(self.address.to_le_bytes());
        buf.extend(self.offset.to_le_bytes());
        buf.extend(self.size.to_le_bytes());
        buf.extend(self.link.to_le_bytes());
        buf.extend(self.info.to_le_bytes());
        buf.extend(self.address_align.to_le_bytes());
        buf.extend(self.entry_size.to_le_bytes());
        buf
    }
}

fn create_elf64_header() -> Header {
    let mut header = Header::default();

    header.ident = u128::from_be_bytes([
        0x7f, 0x45, 0x4c, 0x46, // magic number
        0x02, // class 64
        0x01, // data LSB
        0x01, // version
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    ]);
    header.r#type = 0x01; // REL?
    header.machine = 0x3e; // x86_64
    header.version = 0x01;
    header.section_header_offset = size_of::<Header>() as _;
    header.flags = 0x00;
    header.header_size = size_of::<Header>() as _;
    header.section_header_size = size_of::<SectionHeader>() as _;
    header.section_header_num = 5;
    header.str_index = 2;

    header
}

fn create_null_section() -> SectionHeader {
    SectionHeader::default()
}

fn create_text_section() -> SectionHeader {
    let mut header = SectionHeader::default();
    header.name = 0x01; // data section
    header.r#type = 0x01; // PROGBITS
    header.flags = 0x02 | 0x04; // AX (ALLOC + EXEC)
    header.address_align = 0x01;
    header
}

fn create_strtab_section() -> SectionHeader {
    let mut header = SectionHeader::default();
    header.name = 0x07;
    header.r#type = 0x03; // STRTAB
    header.address_align = 0x01;
    header
}

fn create_symtable_section() -> SectionHeader {
    let mut header = SectionHeader::default();
    header.name = 17;
    header.r#type = 2; // SYMTAB
    header.address_align = 0x08;
    header
}

pub fn generate_elf64() {
    let mut buf = Vec::<u8>::new();

    let header = create_elf64_header();
    buf.extend_from_slice(&header.to_bytes());

    let null_section_header = create_null_section();
    buf.extend_from_slice(&null_section_header.to_bytes());

    let data = b"\0.text\0.shstrtab\0.symtab\0.strtab\0";

    let mut text_section_header = create_text_section();
    text_section_header.offset =
        size_of::<Header>() as u64 + (size_of::<SectionHeader>() as u64 * 5) + data.len() as u64;
    text_section_header.size = 5;
    buf.extend_from_slice(&text_section_header.to_bytes());

    let mut shstrtab_section_header = create_strtab_section();
    shstrtab_section_header.offset =
        size_of::<Header>() as u64 + (size_of::<SectionHeader>() as u64 * 5);
    shstrtab_section_header.size = data.len() as _;
    buf.extend_from_slice(&shstrtab_section_header.to_bytes());

    let mut symtab_section_header = create_symtable_section();
    symtab_section_header.info = 1;
    symtab_section_header.link = 4;
    symtab_section_header.entry_size = 24;
    symtab_section_header.size = 48;
    symtab_section_header.offset = size_of::<Header>() as u64
        + (size_of::<SectionHeader>() as u64 * 5)
        + data.len() as u64
        + b"\xb8\x09\x00\x00\x00".len() as u64;
    buf.extend_from_slice(&symtab_section_header.to_bytes());

    let mut strstab_section_header = create_strtab_section();
    strstab_section_header.name = 25;
    strstab_section_header.offset = size_of::<Header>() as u64
        + (size_of::<SectionHeader>() as u64 * 5)
        + data.len() as u64
        + b"\xb8\x09\x00\x00\x00".len() as u64
        + 48;
    strstab_section_header.size = b".text\0_start\0".len() as _;
    buf.extend_from_slice(&strstab_section_header.to_bytes());

    buf.extend_from_slice(data);
    buf.extend_from_slice(b"\xb8\x09\x00\x00\x00");

    buf.extend_from_slice(&[
        00, 00, 00, 00, 3, 00, 0x1, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00,
    ]);
    buf.extend_from_slice(&[
        06, 00, 00, 00, 0x10, 00, 0x1, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00,
    ]);

    buf.extend_from_slice(b".text\0_start\0");

    println!("{:?}", buf);
}
