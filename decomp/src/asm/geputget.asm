; DOOM-Style Assembly Fallback (RAW BYTE INJECTION)
.386
.model flat
.code
PUBLIC Load_SystemGraphicsAndFonts_
Load_SystemGraphicsAndFonts_ PROC
    db 053h, 051h, 052h, 056h, 083h, 0ech, 034h, 0b8h, 01ch, 07eh, 00ah, 000h, 0e8h, 02bh, 0f7h, 003h
    db 000h, 0a3h, 05ch, 0efh, 01fh, 000h, 085h, 0c0h, 075h, 017h, 068h, 02ch, 07eh, 00ah, 000h, 0e8h
    db 090h, 0f4h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h, 0e8h, 069h, 03ah, 004h
    db 000h, 0b8h, 050h, 07eh, 00ah, 000h, 0e8h, 001h, 0f7h, 003h, 000h, 0a3h, 054h, 0efh, 01fh, 000h
    db 085h, 0c0h, 075h, 017h, 068h, 060h, 07eh, 00ah, 000h, 0e8h, 066h, 0f4h, 0ffh, 0ffh, 083h, 0c4h
    db 004h, 0b8h, 001h, 000h, 000h, 000h, 0e8h, 03fh, 03ah, 004h, 000h, 068h, 084h, 07eh, 00ah, 000h
    db 08dh, 044h, 024h, 004h, 050h, 0e8h, 0abh, 00eh, 004h, 000h, 083h, 0c4h, 008h, 089h, 0e0h, 0e8h
    db 0c8h, 0f6h, 003h, 000h, 0a3h, 000h, 0ech, 01fh, 000h, 085h, 0c0h, 075h, 017h, 068h, 08ch, 07eh
    db 00ah, 000h, 0e8h, 02dh, 0f4h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h, 0e8h
    db 006h, 03ah, 004h, 000h, 0b8h, 0ach, 07eh, 00ah, 000h, 031h, 0d2h, 0e8h, 0ebh, 0fch, 003h, 000h
    db 0a3h, 0a8h, 0c4h, 01ah, 000h, 031h, 0d2h, 0b8h, 0c8h, 07eh, 00ah, 000h, 0e8h, 0dah, 0fch, 003h
    db 000h, 0a3h, 0ach, 0c4h, 01ah, 000h, 031h, 0d2h, 0b8h, 0e4h, 07eh, 00ah, 000h, 0e8h, 0c9h, 0fch
    db 003h, 000h, 0a3h, 0b0h, 0c4h, 01ah, 000h, 031h, 0d2h, 0b8h, 000h, 07fh, 00ah, 000h, 0e8h, 0b8h
    db 0fch, 003h, 000h, 0a3h, 0b4h, 0c4h, 01ah, 000h, 031h, 0d2h, 0b8h, 01ch, 07fh, 00ah, 000h, 0e8h
    db 0a7h, 0fch, 003h, 000h, 0a3h, 0b8h, 0c4h, 01ah, 000h, 031h, 0d2h, 0b8h, 038h, 07fh, 00ah, 000h
    db 0e8h, 096h, 0fch, 003h, 000h, 0a3h, 0bch, 0c4h, 01ah, 000h, 031h, 0d2h, 0b8h, 054h, 07fh, 00ah
    db 000h, 0beh, 001h, 000h, 000h, 000h, 0e8h, 080h, 0fch, 003h, 000h, 0a3h, 0c0h, 0c4h, 01ah, 000h
    db 031h, 0d2h, 0b8h, 064h, 07fh, 00ah, 000h, 031h, 0dbh, 0e8h, 06dh, 0fch, 003h, 000h, 0a3h, 0c4h
    db 0c4h, 01ah, 000h, 031h, 0d2h, 08bh, 08bh, 0a8h, 0c4h, 01ah, 000h, 069h, 0c1h, 03eh, 006h, 000h
    db 000h, 089h, 0b0h, 020h, 0c2h, 024h, 000h, 089h, 0b0h, 024h, 0c2h, 024h, 000h, 083h, 0f9h, 0ffh
    db 075h, 020h, 052h, 068h, 074h, 07fh, 00ah, 000h, 068h, 008h, 0f0h, 01fh, 000h, 0e8h, 062h, 0f3h
    db 0ffh, 0ffh, 083h, 0c4h, 00ch, 089h, 0c8h, 089h, 035h, 00ch, 0eeh, 01fh, 000h, 0e8h, 038h, 039h
    db 004h, 000h, 042h, 083h, 0c3h, 004h, 083h, 0fah, 008h, 07ch, 0bah, 083h, 0c4h, 034h, 05eh, 05ah
    db 059h, 05bh, 0c3h
Load_SystemGraphicsAndFonts_ ENDP

PUBLIC Font_LoadHUDFonts_
Font_LoadHUDFonts_ PROC
    db 053h, 051h, 052h, 056h, 083h, 0ech, 034h, 068h, 0deh, 00bh, 00ch, 000h, 068h, 064h, 085h, 00ah
    db 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h, 066h, 0e6h, 003h, 000h, 083h, 0c4h, 00ch, 068h, 000h
    db 002h, 000h, 000h, 08dh, 044h, 024h, 004h, 050h, 0e8h, 0b2h, 012h, 004h, 000h, 083h, 0c4h, 008h
    db 089h, 0c1h, 085h, 0c0h, 07dh, 01ah, 089h, 0e0h, 050h, 068h, 074h, 085h, 00ah, 000h, 0e8h, 0ddh
    db 0cbh, 0ffh, 0ffh, 083h, 0c4h, 008h, 0b8h, 001h, 000h, 000h, 000h, 0e8h, 0b6h, 011h, 004h, 000h
    db 089h, 0c8h, 0e8h, 0fah, 014h, 004h, 000h, 089h, 0c3h, 089h, 0c6h, 0a1h, 068h, 0eeh, 01fh, 000h
    db 089h, 0dah, 0e8h, 029h, 00dh, 004h, 000h, 08bh, 015h, 010h, 00bh, 00ch, 000h, 001h, 0dah, 0a3h
    db 0f4h, 0efh, 01fh, 000h, 089h, 015h, 010h, 00bh, 00ch, 000h, 085h, 0c0h, 075h, 005h, 0e8h, 085h
    db 0d9h, 002h, 000h, 08bh, 015h, 0f4h, 0efh, 01fh, 000h, 089h, 0f3h, 089h, 0c8h, 0e8h, 0fch, 014h
    db 004h, 000h, 089h, 0c8h, 0e8h, 0f0h, 015h, 004h, 000h, 068h, 0deh, 00bh, 00ch, 000h, 068h, 094h
    db 085h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h, 0d4h, 0e5h, 003h, 000h, 083h, 0c4h, 00ch
    db 031h, 0d2h, 089h, 0e0h, 0e8h, 03eh, 0d4h, 003h, 000h, 0a3h, 00ch, 0eah, 01fh, 000h, 083h, 0f8h
    db 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h, 0e8h, 053h, 0cbh, 0ffh, 0ffh, 083h, 0c4h, 004h
    db 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 02ch, 011h, 004h, 000h, 068h, 0deh, 00bh, 00ch, 000h, 068h
    db 0c0h, 085h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h, 093h, 0e5h, 003h, 000h, 083h, 0c4h
    db 00ch, 031h, 0d2h, 089h, 0e0h, 0e8h, 0fdh, 0d3h, 003h, 000h, 0a3h, 024h, 0eah, 01fh, 000h, 083h
    db 0f8h, 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h, 0e8h, 012h, 0cbh, 0ffh, 0ffh, 083h, 0c4h
    db 004h, 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 0ebh, 010h, 004h, 000h, 068h, 0deh, 00bh, 00ch, 000h
    db 068h, 0d0h, 085h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h, 052h, 0e5h, 003h, 000h, 083h
    db 0c4h, 00ch, 031h, 0d2h, 089h, 0e0h, 0e8h, 0bch, 0d3h, 003h, 000h, 0a3h, 030h, 0eah, 01fh, 000h
    db 083h, 0f8h, 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h, 0e8h, 0d1h, 0cah, 0ffh, 0ffh, 083h
    db 0c4h, 004h, 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 0aah, 010h, 004h, 000h, 068h, 0deh, 00bh, 00ch
    db 000h, 068h, 0e0h, 085h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h, 011h, 0e5h, 003h, 000h
    db 083h, 0c4h, 00ch, 031h, 0d2h, 089h, 0e0h, 0e8h, 07bh, 0d3h, 003h, 000h, 0a3h, 010h, 0eah, 01fh
    db 000h, 083h, 0f8h, 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h, 0e8h, 090h, 0cah, 0ffh, 0ffh
    db 083h, 0c4h, 004h, 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 069h, 010h, 004h, 000h, 068h, 0deh, 00bh
    db 00ch, 000h, 068h, 0f0h, 085h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h, 0d0h, 0e4h, 003h
    db 000h, 083h, 0c4h, 00ch, 031h, 0d2h, 089h, 0e0h, 0e8h, 03ah, 0d3h, 003h, 000h, 0a3h, 028h, 0eah
    db 01fh, 000h, 083h, 0f8h, 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h, 0e8h, 04fh, 0cah, 0ffh
    db 0ffh, 083h, 0c4h, 004h, 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 028h, 010h, 004h, 000h, 068h, 0deh
    db 00bh, 00ch, 000h, 068h, 0fch, 085h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h, 08fh, 0e4h
    db 003h, 000h, 083h, 0c4h, 00ch, 031h, 0d2h, 089h, 0e0h, 0e8h, 0f9h, 0d2h, 003h, 000h, 0a3h, 034h
    db 0eah, 01fh, 000h, 083h, 0f8h, 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h, 0e8h, 00eh, 0cah
    db 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 0e7h, 00fh, 004h, 000h, 068h
    db 0deh, 00bh, 00ch, 000h, 068h, 00ch, 086h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h, 04eh
    db 0e4h, 003h, 000h, 083h, 0c4h, 00ch, 031h, 0d2h, 089h, 0e0h, 0e8h, 0b8h, 0d2h, 003h, 000h, 0a3h
    db 014h, 0eah, 01fh, 000h, 083h, 0f8h, 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h, 0e8h, 0cdh
    db 0c9h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 0a6h, 00fh, 004h, 000h
    db 068h, 0deh, 00bh, 00ch, 000h, 068h, 01ch, 086h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h, 0e8h
    db 00dh, 0e4h, 003h, 000h, 083h, 0c4h, 00ch, 031h, 0d2h, 089h, 0e0h, 0e8h, 077h, 0d2h, 003h, 000h
    db 0a3h, 02ch, 0eah, 01fh, 000h, 083h, 0f8h, 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h, 0e8h
    db 08ch, 0c9h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 065h, 00fh, 004h
    db 000h, 068h, 0deh, 00bh, 00ch, 000h, 068h, 02ch, 086h, 00ah, 000h, 08dh, 044h, 024h, 008h, 050h
    db 0e8h, 0cch, 0e3h, 003h, 000h, 083h, 0c4h, 00ch, 031h, 0d2h, 089h, 0e0h, 0e8h, 036h, 0d2h, 003h
    db 000h, 0a3h, 038h, 0eah, 01fh, 000h, 083h, 0f8h, 0ffh, 075h, 017h, 068h, 0a4h, 085h, 00ah, 000h
    db 0e8h, 04bh, 0c9h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 0ffh, 0ffh, 0ffh, 0ffh, 0e8h, 024h, 00fh
    db 004h, 000h, 083h, 0c4h, 034h, 05eh, 05ah, 059h, 05bh, 0c3h
Font_LoadHUDFonts_ ENDP

PUBLIC Track_LoadOverlayGfx_
Track_LoadOverlayGfx_ PROC
    db 053h, 051h, 052h, 056h, 057h, 055h, 068h, 000h, 002h, 000h, 000h, 068h, 03ch, 086h, 00ah, 000h
    db 0e8h, 0deh, 00fh, 004h, 000h, 083h, 0c4h, 008h, 089h, 0c1h, 085h, 0c0h, 07dh, 017h, 068h, 04ch
    db 086h, 00ah, 000h, 0e8h, 00ch, 0c9h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h
    db 0e8h, 0e5h, 00eh, 004h, 000h, 089h, 0c8h, 0e8h, 029h, 012h, 004h, 000h, 089h, 0c3h, 089h, 0c6h
    db 0a1h, 068h, 0eeh, 01fh, 000h, 089h, 0dah, 0e8h, 058h, 00ah, 004h, 000h, 08bh, 015h, 010h, 00bh
    db 00ch, 000h, 001h, 0dah, 0a3h, 074h, 0efh, 01fh, 000h, 089h, 015h, 010h, 00bh, 00ch, 000h, 085h
    db 0c0h, 075h, 005h, 0e8h, 0b4h, 0d6h, 002h, 000h, 08bh, 015h, 074h, 0efh, 01fh, 000h, 089h, 0f3h
    db 089h, 0c8h, 0e8h, 02bh, 012h, 004h, 000h, 089h, 0c8h, 0e8h, 01fh, 013h, 004h, 000h, 068h, 000h
    db 002h, 000h, 000h, 068h, 078h, 086h, 00ah, 000h, 0e8h, 066h, 00fh, 004h, 000h, 083h, 0c4h, 008h
    db 089h, 0c1h, 085h, 0c0h, 07dh, 017h, 068h, 088h, 086h, 00ah, 000h, 0e8h, 094h, 0c8h, 0ffh, 0ffh
    db 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h, 0e8h, 06dh, 00eh, 004h, 000h, 089h, 0c8h, 0e8h
    db 0b1h, 011h, 004h, 000h, 089h, 0c3h, 089h, 0c6h, 0a1h, 068h, 0eeh, 01fh, 000h, 089h, 0dah, 0e8h
    db 0e0h, 009h, 004h, 000h, 08bh, 03dh, 010h, 00bh, 00ch, 000h, 001h, 0dfh, 0a3h, 068h, 0efh, 01fh
    db 000h, 089h, 03dh, 010h, 00bh, 00ch, 000h, 085h, 0c0h, 075h, 005h, 0e8h, 03ch, 0d6h, 002h, 000h
    db 08bh, 015h, 068h, 0efh, 01fh, 000h, 089h, 0f3h, 089h, 0c8h, 0e8h, 0b3h, 011h, 004h, 000h, 089h
    db 0c8h, 0e8h, 0a7h, 012h, 004h, 000h, 068h, 000h, 002h, 000h, 000h, 068h, 0b4h, 086h, 00ah, 000h
    db 0e8h, 0eeh, 00eh, 004h, 000h, 083h, 0c4h, 008h, 089h, 0c1h, 085h, 0c0h, 07dh, 017h, 068h, 0c0h
    db 086h, 00ah, 000h, 0e8h, 01ch, 0c8h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h
    db 0e8h, 0f5h, 00dh, 004h, 000h, 089h, 0c8h, 0e8h, 039h, 011h, 004h, 000h, 089h, 0c3h, 089h, 0c6h
    db 0a1h, 068h, 0eeh, 01fh, 000h, 089h, 0dah, 0e8h, 068h, 009h, 004h, 000h, 08bh, 02dh, 010h, 00bh
    db 00ch, 000h, 001h, 0ddh, 0a3h, 070h, 0efh, 01fh, 000h, 089h, 02dh, 010h, 00bh, 00ch, 000h, 085h
    db 0c0h, 075h, 005h, 0e8h, 0c4h, 0d5h, 002h, 000h, 08bh, 015h, 070h, 0efh, 01fh, 000h, 089h, 0f3h
    db 089h, 0c8h, 0e8h, 03bh, 011h, 004h, 000h, 089h, 0c8h, 0e8h, 02fh, 012h, 004h, 000h, 068h, 000h
    db 002h, 000h, 000h, 068h, 0e8h, 086h, 00ah, 000h, 0e8h, 076h, 00eh, 004h, 000h, 083h, 0c4h, 008h
    db 089h, 0c1h, 085h, 0c0h, 07dh, 017h, 068h, 0f4h, 086h, 00ah, 000h, 0e8h, 0a4h, 0c7h, 0ffh, 0ffh
    db 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h, 0e8h, 07dh, 00dh, 004h, 000h, 089h, 0c8h, 0e8h
    db 0c1h, 010h, 004h, 000h, 089h, 0c3h, 089h, 0c6h, 0a1h, 068h, 0eeh, 01fh, 000h, 089h, 0dah, 0e8h
    db 0f0h, 008h, 004h, 000h, 08bh, 015h, 010h, 00bh, 00ch, 000h, 001h, 0dah, 0a3h, 06ch, 0efh, 01fh
    db 000h, 089h, 015h, 010h, 00bh, 00ch, 000h, 085h, 0c0h, 075h, 005h, 0e8h, 04ch, 0d5h, 002h, 000h
    db 08bh, 015h, 06ch, 0efh, 01fh, 000h, 089h, 0f3h, 089h, 0c8h, 0e8h, 0c3h, 010h, 004h, 000h, 089h
    db 0c8h, 0e8h, 0b7h, 011h, 004h, 000h, 068h, 000h, 002h, 000h, 000h, 068h, 01ch, 087h, 00ah, 000h
    db 0e8h, 0feh, 00dh, 004h, 000h, 083h, 0c4h, 008h, 089h, 0c1h, 085h, 0c0h, 07dh, 017h, 068h, 028h
    db 087h, 00ah, 000h, 0e8h, 02ch, 0c7h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h
    db 0e8h, 005h, 00dh, 004h, 000h, 089h, 0c8h, 0e8h, 049h, 010h, 004h, 000h, 089h, 0c3h, 089h, 0c6h
    db 0a1h, 068h, 0eeh, 01fh, 000h, 089h, 0dah, 0e8h, 078h, 008h, 004h, 000h, 08bh, 03dh, 010h, 00bh
    db 00ch, 000h, 001h, 0dfh, 0a3h, 03ch, 0efh, 01fh, 000h, 089h, 03dh, 010h, 00bh, 00ch, 000h, 085h
    db 0c0h, 075h, 005h, 0e8h, 0d4h, 0d4h, 002h, 000h, 08bh, 015h, 03ch, 0efh, 01fh, 000h, 089h, 0f3h
    db 089h, 0c8h, 0e8h, 04bh, 010h, 004h, 000h, 089h, 0c8h, 0e8h, 03fh, 011h, 004h, 000h, 068h, 000h
    db 002h, 000h, 000h, 068h, 050h, 087h, 00ah, 000h, 0e8h, 086h, 00dh, 004h, 000h, 083h, 0c4h, 008h
    db 089h, 0c1h, 085h, 0c0h, 07dh, 017h, 068h, 05ch, 087h, 00ah, 000h, 0e8h, 0b4h, 0c6h, 0ffh, 0ffh
    db 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h, 0e8h, 08dh, 00ch, 004h, 000h, 089h, 0c8h, 0e8h
    db 0d1h, 00fh, 004h, 000h, 089h, 0c3h, 089h, 0c6h, 0a1h, 068h, 0eeh, 01fh, 000h, 089h, 0dah, 0e8h
    db 000h, 008h, 004h, 000h, 08bh, 02dh, 010h, 00bh, 00ch, 000h, 001h, 0ddh, 0a3h, 034h, 0efh, 01fh
    db 000h, 089h, 02dh, 010h, 00bh, 00ch, 000h, 085h, 0c0h, 075h, 005h, 0e8h, 05ch, 0d4h, 002h, 000h
    db 08bh, 015h, 034h, 0efh, 01fh, 000h, 089h, 0f3h, 089h, 0c8h, 0e8h, 0d3h, 00fh, 004h, 000h, 089h
    db 0c8h, 0e8h, 0c7h, 010h, 004h, 000h, 068h, 000h, 002h, 000h, 000h, 068h, 084h, 087h, 00ah, 000h
    db 0e8h, 00eh, 00dh, 004h, 000h, 083h, 0c4h, 008h, 089h, 0c1h, 085h, 0c0h, 07dh, 017h, 068h, 090h
    db 087h, 00ah, 000h, 0e8h, 03ch, 0c6h, 0ffh, 0ffh, 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h
    db 0e8h, 015h, 00ch, 004h, 000h, 089h, 0c8h, 0e8h, 059h, 00fh, 004h, 000h, 089h, 0c3h, 089h, 0c6h
    db 0a1h, 068h, 0eeh, 01fh, 000h, 089h, 0dah, 0e8h, 088h, 007h, 004h, 000h, 08bh, 015h, 010h, 00bh
    db 00ch, 000h, 001h, 0dah, 0a3h, 030h, 0efh, 01fh, 000h, 089h, 015h, 010h, 00bh, 00ch, 000h, 085h
    db 0c0h, 075h, 005h, 0e8h, 0e4h, 0d3h, 002h, 000h, 08bh, 015h, 030h, 0efh, 01fh, 000h, 089h, 0f3h
    db 089h, 0c8h, 0e8h, 05bh, 00fh, 004h, 000h, 089h, 0c8h, 0e8h, 04fh, 010h, 004h, 000h, 068h, 000h
    db 002h, 000h, 000h, 068h, 0b8h, 087h, 00ah, 000h, 0e8h, 096h, 00ch, 004h, 000h, 083h, 0c4h, 008h
    db 089h, 0c1h, 085h, 0c0h, 07dh, 017h, 068h, 0c4h, 087h, 00ah, 000h, 0e8h, 0c4h, 0c5h, 0ffh, 0ffh
    db 083h, 0c4h, 004h, 0b8h, 001h, 000h, 000h, 000h, 0e8h, 09dh, 00bh, 004h, 000h, 089h, 0c8h, 0e8h
    db 0e1h, 00eh, 004h, 000h, 089h, 0c3h, 089h, 0c6h, 0a1h, 068h, 0eeh, 01fh, 000h, 089h, 0dah, 0e8h
    db 010h, 007h, 004h, 000h, 08bh, 03dh, 010h, 00bh, 00ch, 000h, 001h, 0dfh, 0a3h, 048h, 0efh, 01fh
    db 000h, 089h, 03dh, 010h, 00bh, 00ch, 000h, 085h, 0c0h, 075h, 005h, 0e8h, 06ch, 0d3h, 002h, 000h
    db 08bh, 015h, 048h, 0efh, 01fh, 000h, 089h, 0f3h, 089h, 0c8h, 0e8h, 0e3h, 00eh, 004h, 000h, 089h
    db 0c8h, 0e8h, 0d7h, 00fh, 004h, 000h, 05dh, 05fh, 05eh, 05ah, 059h, 05bh, 0c3h
Track_LoadOverlayGfx_ ENDP

PUBLIC Font_DrawHUDText_
Font_DrawHUDText_ PROC
    db 0edh, 075h, 067h, 08bh, 015h, 0ebh, 0f6h, 01fh, 000h, 08dh, 004h, 0d5h, 000h, 000h, 000h, 000h
    db 029h, 0d0h, 001h, 0c0h, 089h, 0c2h, 0c1h, 0e0h, 004h, 029h, 0d0h, 08bh, 00dh, 0ach, 0c4h, 01ah
    db 000h, 005h, 0deh, 00eh, 00ch, 000h, 089h, 0cah, 083h, 0c0h, 01eh, 0e8h, 04bh, 0d9h, 001h, 000h
    db 08bh, 015h, 048h, 0edh, 01fh, 000h, 031h, 0c9h, 085h, 0d2h, 07eh, 02eh, 08dh, 070h, 067h, 06bh
    db 0c1h, 01ah, 0bbh, 000h, 01bh, 000h, 000h, 089h, 0e2h, 001h, 0f0h, 089h, 05ch, 024h, 004h, 0c1h
    db 0e0h, 008h, 031h, 0dbh, 089h, 004h, 024h, 0a1h, 0d0h, 0ebh, 01fh, 000h, 041h, 0e8h, 0a6h, 01bh
    db 001h, 000h, 03bh, 00dh, 048h, 0edh, 01fh, 000h, 07ch, 0d5h, 0a1h, 000h, 0f0h, 01fh, 000h, 050h
    db 068h, 040h, 001h, 000h, 000h, 08bh, 015h, 038h, 0efh, 01fh, 000h, 0a1h, 0fch, 0efh, 01fh, 000h
    db 052h, 089h, 0c2h, 0c1h, 0fah, 01fh, 02bh, 0c2h, 0d1h, 0f8h, 083h, 0e8h, 032h, 050h, 0a1h, 000h
    db 0f0h, 01fh, 000h, 089h, 0c2h, 0c1h, 0fah, 01fh, 02bh, 0c2h, 0d1h, 0f8h, 02dh, 0a0h, 000h, 000h
    db 000h, 050h, 068h, 060h, 0bbh, 00eh, 000h, 0b9h, 040h, 001h, 000h, 000h, 031h, 0dbh, 06ah, 064h
    db 031h, 0d2h, 089h, 0f8h, 0e8h, 0ffh, 0d9h, 000h, 000h, 06ah, 008h, 0b8h, 060h, 0bbh, 00eh, 000h
    db 08bh, 015h, 000h, 0f0h, 01fh, 000h, 08bh, 00dh, 0fch, 0efh, 01fh, 000h, 089h, 0d3h, 0e8h, 015h
    db 01bh, 001h, 000h, 0a1h, 038h, 0efh, 01fh, 000h, 0a3h, 048h, 07eh, 025h, 000h, 083h, 0c4h, 020h
    db 05dh, 05fh, 05eh, 05ah, 059h, 05bh, 0c3h
Font_DrawHUDText_ ENDP

PUBLIC Font_InitSystem_
Font_InitSystem_ PROC
    db 068h, 024h, 000h, 000h, 000h, 0e8h, 062h, 05eh, 001h, 000h, 053h, 051h, 052h, 056h, 057h, 055h
    db 089h, 0e5h, 081h, 0ech, 008h, 000h, 000h, 000h, 083h, 03dh, 060h, 07ch, 00dh, 000h, 001h, 075h
    db 00ch, 0c7h, 045h, 0fch, 0f2h, 003h, 000h, 000h, 0e9h, 0c2h, 000h, 000h, 000h, 0c7h, 005h, 060h
    db 07ch, 00dh, 000h, 001h, 000h, 000h, 000h, 0e8h, 07ch, 041h, 0ffh, 0ffh, 0a3h, 010h, 0c2h, 024h
    db 000h, 0c7h, 005h, 030h, 09ch, 020h, 000h, 064h, 07ch, 00dh, 000h, 0c7h, 005h, 034h, 09ch, 020h
    db 000h, 019h, 013h, 006h, 000h, 0c7h, 005h, 038h, 09ch, 020h, 000h, 000h, 000h, 000h, 000h, 0a1h
    db 010h, 0c2h, 024h, 000h, 0e8h, 013h, 043h, 0ffh, 0ffh, 0c7h, 045h, 0f8h, 000h, 000h, 000h, 000h
    db 083h, 07dh, 0f8h, 01eh, 07ch, 00ah, 0ebh, 070h, 08bh, 045h, 0f8h, 0ffh, 045h, 0f8h, 0ebh, 0f0h
    db 069h, 045h, 0f8h, 03eh, 006h, 000h, 000h, 0c7h, 080h, 014h, 0c2h, 024h, 000h, 000h, 000h, 000h
    db 000h, 069h, 045h, 0f8h, 03eh, 006h, 000h, 000h, 0c7h, 080h, 018h, 0c2h, 024h, 000h, 000h, 000h
    db 000h, 000h, 069h, 045h, 0f8h, 03eh, 006h, 000h, 000h, 0c7h, 080h, 01ch, 0c2h, 024h, 000h, 000h
    db 000h, 000h, 000h, 069h, 045h, 0f8h, 03eh, 006h, 000h, 000h, 0c7h, 080h, 020h, 0c2h, 024h, 000h
    db 000h, 000h, 000h, 000h, 069h, 045h, 0f8h, 03eh, 006h, 000h, 000h, 0c7h, 080h, 024h, 0c2h, 024h
    db 000h, 001h, 000h, 000h, 000h, 069h, 045h, 0f8h, 03eh, 006h, 000h, 000h, 0c7h, 080h, 028h, 0c2h
    db 024h, 000h, 001h, 000h, 000h, 000h, 0ebh, 090h, 0c7h, 045h, 0fch, 001h, 000h, 000h, 000h, 08bh
    db 045h, 0fch, 0c9h, 05fh, 05eh, 05ah, 059h, 05bh, 0c3h
Font_InitSystem_ ENDP

PUBLIC Font_Shutdown_
Font_Shutdown_ PROC
    db 068h, 024h, 000h, 000h, 000h, 0e8h, 069h, 05dh, 001h, 000h, 053h, 051h, 052h, 056h, 057h, 055h
    db 089h, 0e5h, 081h, 0ech, 008h, 000h, 000h, 000h, 083h, 03dh, 060h, 07ch, 00dh, 000h, 000h, 075h
    db 009h, 0c7h, 045h, 0fch, 0fch, 003h, 000h, 000h, 0ebh, 04ch, 0a1h, 010h, 0c2h, 024h, 000h, 0e8h
    db 0f7h, 042h, 0ffh, 0ffh, 0c7h, 005h, 060h, 07ch, 00dh, 000h, 000h, 000h, 000h, 000h, 0c7h, 045h
    db 0f8h, 000h, 000h, 000h, 000h, 083h, 07dh, 0f8h, 01eh, 07ch, 00ah, 0ebh, 022h, 08bh, 045h, 0f8h
    db 0ffh, 045h, 0f8h, 0ebh, 0f0h, 069h, 045h, 0f8h, 03eh, 006h, 000h, 000h, 083h, 0b8h, 014h, 0c2h
    db 024h, 000h, 001h, 075h, 008h, 08bh, 045h, 0f8h, 0e8h, 0cdh, 002h, 000h, 000h, 0ebh, 0deh, 0c7h
    db 045h, 0fch, 001h, 000h, 000h, 000h, 08bh, 045h, 0fch, 0c9h, 05fh, 05eh, 05ah, 059h, 05bh, 0c3h
Font_Shutdown_ ENDP

PUBLIC Font_Parse_
Font_Parse_ PROC
    db 068h, 054h, 000h, 000h, 000h, 0e8h, 0e9h, 05ch, 001h, 000h, 053h, 051h, 056h, 057h, 055h, 089h
    db 0e5h, 081h, 0ech, 03ch, 000h, 000h, 000h, 089h, 045h, 0e4h, 089h, 055h, 0e8h, 083h, 03dh, 060h
    db 07ch, 00dh, 000h, 000h, 075h, 005h, 0e8h, 05ch, 0feh, 0ffh, 0ffh, 08bh, 045h, 0e4h, 089h, 045h
    db 0f4h, 0bah, 06ch, 0aeh, 00ah, 000h, 08bh, 045h, 0f4h, 0e8h, 0f9h, 05ch, 001h, 000h, 085h, 0c0h
    db 074h, 016h, 0c7h, 005h, 0f4h, 05ch, 00ch, 000h, 01ah, 004h, 000h, 000h, 0c7h, 045h, 0fch, 0ffh
    db 0ffh, 0ffh, 0ffh, 0e9h, 0f1h, 001h, 000h, 000h, 08bh, 045h, 0f4h, 066h, 083h, 078h, 004h, 064h
    db 074h, 016h, 0c7h, 005h, 0f4h, 05ch, 00ch, 000h, 024h, 004h, 000h, 000h, 0c7h, 045h, 0fch, 0ffh
    db 0ffh, 0ffh, 0ffh, 0e9h, 0d1h, 001h, 000h, 000h, 0c7h, 045h, 0f8h, 000h, 000h, 000h, 000h, 083h
    db 07dh, 0f8h, 01eh, 07ch, 00ah, 0ebh, 018h, 08bh, 045h, 0f8h, 0ffh, 045h, 0f8h, 0ebh, 0f0h, 069h
    db 045h, 0f8h, 03eh, 006h, 000h, 000h, 083h, 0b8h, 014h, 0c2h, 024h, 000h, 000h, 075h, 0e8h, 083h
    db 07dh, 0f8h, 01eh, 075h, 016h, 0c7h, 005h, 0f4h, 05ch, 00ch, 000h, 006h, 004h, 000h, 000h, 0c7h
    db 045h, 0fch, 0ffh, 0ffh, 0ffh, 0ffh, 0e9h, 08eh, 001h, 000h, 000h, 08bh, 045h, 0f8h, 089h, 045h
    db 0ech, 069h, 055h, 0ech, 03eh, 006h, 000h, 000h, 08bh, 045h, 0f4h, 066h, 08bh, 040h, 006h, 066h
    db 089h, 082h, 02ch, 0c2h, 024h, 000h, 069h, 055h, 0ech, 03eh, 006h, 000h, 000h, 08bh, 045h, 0f4h
    db 066h, 08bh, 040h, 008h, 066h, 089h, 082h, 02eh, 0c2h, 024h, 000h, 069h, 055h, 0ech, 03eh, 006h
    db 000h, 000h, 08bh, 045h, 0f4h, 066h, 08bh, 040h, 00ah, 066h, 089h, 082h, 030h, 0c2h, 024h, 000h
    db 0c7h, 045h, 0f8h, 000h, 000h, 000h, 000h, 081h, 07dh, 0f8h, 0e0h, 000h, 000h, 000h, 07ch, 00ah
    db 0ebh, 02eh, 08bh, 045h, 0f8h, 0ffh, 045h, 0f8h, 0ebh, 0edh, 08bh, 055h, 0f8h, 001h, 0d2h, 003h
    db 055h, 0f4h, 069h, 045h, 0ech, 03eh, 006h, 000h, 000h, 08bh, 05dh, 0f8h, 001h, 0dbh, 001h, 0d8h
    db 066h, 08bh, 092h, 08ch, 006h, 000h, 000h, 066h, 089h, 090h, 092h, 0c6h, 024h, 000h, 0ebh, 0d2h
    db 08bh, 045h, 0f4h, 005h, 04ch, 008h, 000h, 000h, 089h, 045h, 0f0h, 0c7h, 045h, 0f8h, 000h, 000h
    db 000h, 000h, 081h, 07dh, 0f8h, 0e0h, 000h, 000h, 000h, 07ch, 00dh, 0e9h, 0d2h, 000h, 000h, 000h
    db 08bh, 045h, 0f8h, 0ffh, 045h, 0f8h, 0ebh, 0eah, 08bh, 045h, 0f8h, 0c1h, 0e0h, 002h, 003h, 045h
    db 0f4h, 083h, 078h, 00ch, 0ffh, 075h, 02fh, 069h, 045h, 0ech, 03eh, 006h, 000h, 000h, 003h, 045h
    db 0f8h, 0c6h, 080h, 032h, 0c2h, 024h, 000h, 000h, 069h, 045h, 0ech, 03eh, 006h, 000h, 000h, 08bh
    db 055h, 0f8h, 0c1h, 0e2h, 002h, 001h, 0d0h, 0c7h, 080h, 012h, 0c3h, 024h, 000h, 000h, 000h, 000h
    db 000h, 0e9h, 087h, 000h, 000h, 000h, 069h, 045h, 0ech, 03eh, 006h, 000h, 000h, 003h, 045h, 0f8h
    db 0c6h, 080h, 032h, 0c2h, 024h, 000h, 001h, 08bh, 045h, 0f8h, 001h, 0c0h, 003h, 045h, 0f4h, 00fh
    db 0bfh, 080h, 08ch, 006h, 000h, 000h, 089h, 045h, 0c8h, 08bh, 045h, 0f4h, 00fh, 0bfh, 040h, 00ah
    db 089h, 045h, 0cch, 0c7h, 045h, 0d0h, 000h, 000h, 000h, 000h, 08bh, 055h, 0f8h, 0c1h, 0e2h, 002h
    db 003h, 055h, 0f4h, 08bh, 045h, 0f0h, 003h, 042h, 00ch, 089h, 045h, 0d4h, 08bh, 045h, 0f8h, 001h
    db 0c0h, 003h, 045h, 0f4h, 00fh, 0bfh, 080h, 08ch, 006h, 000h, 000h, 089h, 045h, 0d8h, 0c7h, 045h
    db 0dch, 000h, 000h, 000h, 000h, 0c7h, 045h, 0e0h, 000h, 000h, 000h, 000h, 031h, 0d2h, 08dh, 045h
    db 0c4h, 0e8h, 0c1h, 043h, 0ffh, 0ffh, 089h, 0c2h, 069h, 05dh, 0ech, 03eh, 006h, 000h, 000h, 08bh
    db 045h, 0f8h, 0c1h, 0e0h, 002h, 001h, 0d8h, 089h, 090h, 012h, 0c3h, 024h, 000h, 0e9h, 02eh, 0ffh
    db 0ffh, 0ffh, 069h, 045h, 0ech, 03eh, 006h, 000h, 000h, 0c7h, 080h, 014h, 0c2h, 024h, 000h, 001h
    db 000h, 000h, 000h, 08bh, 045h, 0ech, 089h, 045h, 0fch, 08bh, 045h, 0fch, 0c9h, 05fh, 05eh, 059h
    db 05bh, 0c3h
Font_Parse_ ENDP

PUBLIC Font_Load_
Font_Load_ PROC
    db 068h, 02ch, 000h, 000h, 000h, 0e8h, 097h, 05ah, 001h, 000h, 053h, 051h, 056h, 057h, 055h, 089h
    db 0e5h, 081h, 0ech, 014h, 000h, 000h, 000h, 089h, 045h, 0ech, 089h, 055h, 0f0h, 08bh, 045h, 0ech
    db 0e8h, 08ch, 0f9h, 0ffh, 0ffh, 089h, 045h, 0f4h, 083h, 07dh, 0f4h, 000h, 075h, 013h, 0c7h, 005h
    db 0f4h, 05ch, 00ch, 000h, 0e8h, 003h, 000h, 000h, 0c7h, 045h, 0fch, 0ffh, 0ffh, 0ffh, 0ffh, 0ebh
    db 01eh, 08bh, 055h, 0f0h, 08bh, 045h, 0f4h, 0e8h, 062h, 0fdh, 0ffh, 0ffh, 089h, 045h, 0f8h, 08bh
    db 055h, 0f4h, 031h, 0c0h, 0e8h, 0b4h, 03ah, 000h, 000h, 08bh, 045h, 0f8h, 089h, 045h, 0fch, 08bh
    db 045h, 0fch, 0c9h, 05fh, 05eh, 059h, 05bh, 0c3h
Font_Load_ ENDP

PUBLIC Font_Unload_
Font_Unload_ PROC
    db 068h, 028h, 000h, 000h, 000h, 0e8h, 02fh, 05ah, 001h, 000h, 053h, 051h, 052h, 056h, 057h, 055h
    db 089h, 0e5h, 081h, 0ech, 00ch, 000h, 000h, 000h, 089h, 045h, 0f4h, 069h, 045h, 0f4h, 03eh, 006h
    db 000h, 000h, 0c7h, 080h, 014h, 0c2h, 024h, 000h, 000h, 000h, 000h, 000h, 0c7h, 045h, 0f8h, 000h
    db 000h, 000h, 000h, 081h, 07dh, 0f8h, 0e0h, 000h, 000h, 000h, 07ch, 00ah, 0ebh, 039h, 08bh, 045h
    db 0f8h, 0ffh, 045h, 0f8h, 0ebh, 0edh, 069h, 045h, 0f4h, 03eh, 006h, 000h, 000h, 003h, 045h, 0f8h
    db 080h, 0b8h, 032h, 0c2h, 024h, 000h, 001h, 075h, 01ch, 069h, 045h, 0f4h, 03eh, 006h, 000h, 000h
    db 08bh, 055h, 0f8h, 0c1h, 0e2h, 002h, 001h, 0d0h, 08bh, 090h, 012h, 0c3h, 024h, 000h, 031h, 0c0h
    db 0e8h, 0a8h, 042h, 0ffh, 0ffh, 0ebh, 0c7h, 0c7h, 045h, 0fch, 001h, 000h, 000h, 000h, 08bh, 045h
    db 0fch, 0c9h, 05fh, 05eh, 05ah, 059h, 05bh, 0c3h
Font_Unload_ ENDP

PUBLIC Font_GetTextWidth_
Font_GetTextWidth_ PROC
    db 068h, 040h, 000h, 000h, 000h, 0e8h, 0a7h, 059h, 001h, 000h, 053h, 051h, 056h, 057h, 055h, 089h
    db 0e5h, 081h, 0ech, 028h, 000h, 000h, 000h, 089h, 045h, 0dch, 089h, 055h, 0e0h, 0c7h, 045h, 0f8h
    db 000h, 000h, 000h, 000h, 08bh, 045h, 0f8h, 089h, 045h, 0f4h, 083h, 07dh, 0e0h, 01eh, 07dh, 010h
    db 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h, 083h, 0b8h, 014h, 0c2h, 024h, 000h, 000h, 075h, 00ah
    db 0c7h, 005h, 0f4h, 05ch, 00ch, 000h, 010h, 004h, 000h, 000h, 069h, 045h, 0e0h, 03eh, 006h, 000h
    db 000h, 083h, 0b8h, 01ch, 0c2h, 024h, 000h, 000h, 074h, 00ch, 0c7h, 045h, 0f0h, 002h, 000h, 000h
    db 000h, 0e9h, 00fh, 002h, 000h, 000h, 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 080h
    db 02eh, 0c2h, 024h, 000h, 0c1h, 0e0h, 008h, 089h, 045h, 0d8h, 0dbh, 045h, 0d8h, 0dch, 00dh, 070h
    db 0aeh, 00ah, 000h, 0e8h, 039h, 048h, 0ffh, 0ffh, 0dbh, 05dh, 0e8h, 083h, 07dh, 0e8h, 001h, 07dh
    db 007h, 0c7h, 045h, 0e8h, 001h, 000h, 000h, 000h, 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h, 083h
    db 0b8h, 020h, 0c2h, 024h, 000h, 000h, 00fh, 085h, 00bh, 001h, 000h, 000h, 0c7h, 045h, 0ech, 000h
    db 000h, 000h, 000h, 08bh, 045h, 0dch, 0e8h, 0dbh, 059h, 001h, 000h, 03bh, 045h, 0ech, 07fh, 00dh
    db 0e9h, 0e4h, 000h, 000h, 000h, 08bh, 045h, 0ech, 0ffh, 045h, 0ech, 0ebh, 0e6h, 08bh, 045h, 0dch
    db 003h, 045h, 0ech, 08ah, 000h, 088h, 045h, 0fch, 080h, 07dh, 0fch, 000h, 00fh, 084h, 0c7h, 000h
    db 000h, 000h, 069h, 055h, 0e0h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 045h, 0fch, 001h, 0d0h, 080h
    db 0b8h, 012h, 0c2h, 024h, 000h, 001h, 00fh, 085h, 07fh, 000h, 000h, 000h, 0c7h, 045h, 0e4h, 000h
    db 000h, 000h, 000h, 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 055h, 0fch, 001h, 0d2h
    db 001h, 0d0h, 069h, 055h, 0e0h, 03eh, 006h, 000h, 000h, 066h, 08bh, 080h, 052h, 0c6h, 024h, 000h
    db 066h, 03bh, 082h, 02eh, 0c2h, 024h, 000h, 074h, 032h, 069h, 055h, 0e0h, 03eh, 006h, 000h, 000h
    db 00fh, 0b6h, 045h, 0fch, 001h, 0c0h, 001h, 0c2h, 00fh, 0bfh, 082h, 052h, 0c6h, 024h, 000h, 069h
    db 055h, 0e0h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 092h, 02eh, 0c2h, 024h, 000h, 029h, 0c2h, 089h
    db 0d0h, 0c1h, 0fah, 01fh, 02bh, 0c2h, 0d1h, 0f8h, 089h, 045h, 0e4h, 069h, 045h, 0e0h, 03eh, 006h
    db 000h, 000h, 00fh, 0bfh, 090h, 02eh, 0c2h, 024h, 000h, 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h
    db 003h, 090h, 024h, 0c2h, 024h, 000h, 001h, 055h, 0f8h, 0ebh, 029h, 080h, 07dh, 0fch, 020h, 075h
    db 023h, 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 090h, 02eh, 0c2h, 024h, 000h, 069h
    db 045h, 0e0h, 03eh, 006h, 000h, 000h, 003h, 090h, 024h, 0c2h, 024h, 000h, 001h, 055h, 0f8h, 0e9h
    db 021h, 0ffh, 0ffh, 0ffh, 0e9h, 01ch, 0ffh, 0ffh, 0ffh, 08bh, 045h, 0f8h, 02bh, 045h, 0f4h, 089h
    db 045h, 0f0h, 0e9h, 0beh, 000h, 000h, 000h, 0c7h, 045h, 0ech, 000h, 000h, 000h, 000h, 08bh, 045h
    db 0dch, 0e8h, 0d0h, 058h, 001h, 000h, 03bh, 045h, 0ech, 07fh, 00dh, 0e9h, 09ch, 000h, 000h, 000h
    db 08bh, 045h, 0ech, 0ffh, 045h, 0ech, 0ebh, 0e6h, 08bh, 045h, 0dch, 003h, 045h, 0ech, 08ah, 000h
    db 088h, 045h, 0fch, 080h, 07dh, 0fch, 000h, 00fh, 084h, 07fh, 000h, 000h, 000h, 069h, 055h, 0e0h
    db 03eh, 006h, 000h, 000h, 00fh, 0b6h, 045h, 0fch, 001h, 0d0h, 080h, 0b8h, 012h, 0c2h, 024h, 000h
    db 001h, 075h, 028h, 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 055h, 0fch, 001h, 0d2h
    db 001h, 0d0h, 00fh, 0bfh, 090h, 052h, 0c6h, 024h, 000h, 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h
    db 003h, 090h, 024h, 0c2h, 024h, 000h, 001h, 055h, 0f8h, 0ebh, 03ch, 080h, 07dh, 0fch, 020h, 075h
    db 036h, 069h, 045h, 0e0h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 080h, 02eh, 0c2h, 024h, 000h, 0c1h
    db 0e0h, 008h, 089h, 045h, 0d8h, 0dbh, 045h, 0d8h, 0dch, 00dh, 078h, 0aeh, 00ah, 000h, 0dch, 00dh
    db 080h, 0aeh, 00ah, 000h, 0e8h, 068h, 046h, 0ffh, 0ffh, 0dbh, 05dh, 0d8h, 08bh, 045h, 0d8h, 001h
    db 045h, 0f8h, 0e9h, 069h, 0ffh, 0ffh, 0ffh, 0e9h, 064h, 0ffh, 0ffh, 0ffh, 08bh, 045h, 0f8h, 02bh
    db 045h, 0f4h, 089h, 045h, 0f0h, 08bh, 045h, 0f0h, 0c9h, 05fh, 05eh, 059h, 05bh, 0c3h
Font_GetTextWidth_ ENDP

PUBLIC Font_DrawText_
Font_DrawText_ PROC
    db 068h, 048h, 000h, 000h, 000h, 0e8h, 029h, 057h, 001h, 000h, 056h, 057h, 055h, 089h, 0e5h, 081h
    db 0ech, 038h, 000h, 000h, 000h, 089h, 045h, 0d8h, 089h, 055h, 0e4h, 089h, 05dh, 0dch, 089h, 04dh
    db 0e0h, 08bh, 045h, 0dch, 089h, 045h, 0f4h, 083h, 07dh, 0e4h, 01eh, 07dh, 010h, 069h, 045h, 0e4h
    db 03eh, 006h, 000h, 000h, 083h, 0b8h, 014h, 0c2h, 024h, 000h, 000h, 075h, 00ch, 0c7h, 045h, 0f0h
    db 010h, 004h, 000h, 000h, 0e9h, 0ffh, 004h, 000h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h
    db 083h, 0b8h, 01ch, 0c2h, 024h, 000h, 000h, 074h, 00ch, 0c7h, 045h, 0f0h, 002h, 000h, 000h, 000h
    db 0e9h, 0e3h, 004h, 000h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 080h, 02eh
    db 0c2h, 024h, 000h, 0c1h, 0e0h, 008h, 089h, 045h, 0c8h, 0dbh, 045h, 0c8h, 0dch, 00dh, 088h, 0aeh
    db 00ah, 000h, 0e8h, 0bch, 045h, 0ffh, 0ffh, 0dbh, 05dh, 0e8h, 083h, 07dh, 0e8h, 001h, 07dh, 007h
    db 0c7h, 045h, 0e8h, 001h, 000h, 000h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 083h, 0b8h
    db 020h, 0c2h, 024h, 000h, 000h, 00fh, 085h, 097h, 002h, 000h, 000h, 069h, 045h, 0e4h, 03eh, 006h
    db 000h, 000h, 083h, 0b8h, 018h, 0c2h, 024h, 000h, 000h, 00fh, 084h, 04ch, 001h, 000h, 000h, 0c7h
    db 045h, 0ech, 000h, 000h, 000h, 000h, 08bh, 045h, 0d8h, 0e8h, 04ah, 057h, 001h, 000h, 03bh, 045h
    db 0ech, 07fh, 00dh, 0e9h, 0e4h, 000h, 000h, 000h, 08bh, 045h, 0ech, 0ffh, 045h, 0ech, 0ebh, 0e6h
    db 08bh, 045h, 0d8h, 003h, 045h, 0ech, 08ah, 000h, 088h, 045h, 0fch, 080h, 07dh, 0fch, 000h, 00fh
    db 084h, 0c7h, 000h, 000h, 000h, 069h, 055h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 045h, 0fch
    db 001h, 0d0h, 080h, 0b8h, 012h, 0c2h, 024h, 000h, 001h, 00fh, 085h, 07fh, 000h, 000h, 000h, 0c7h
    db 045h, 0f8h, 000h, 000h, 000h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 055h
    db 0fch, 001h, 0d2h, 001h, 0d0h, 069h, 055h, 0e4h, 03eh, 006h, 000h, 000h, 066h, 08bh, 080h, 052h
    db 0c6h, 024h, 000h, 066h, 03bh, 082h, 02eh, 0c2h, 024h, 000h, 074h, 032h, 069h, 055h, 0e4h, 03eh
    db 006h, 000h, 000h, 00fh, 0b6h, 045h, 0fch, 001h, 0c0h, 001h, 0c2h, 00fh, 0bfh, 082h, 052h, 0c6h
    db 024h, 000h, 069h, 055h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 092h, 02eh, 0c2h, 024h, 000h
    db 029h, 0c2h, 089h, 0d0h, 0c1h, 0fah, 01fh, 02bh, 0c2h, 0d1h, 0f8h, 089h, 045h, 0f8h, 069h, 045h
    db 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 090h, 02eh, 0c2h, 024h, 000h, 069h, 045h, 0e4h, 03eh
    db 006h, 000h, 000h, 003h, 090h, 024h, 0c2h, 024h, 000h, 001h, 055h, 0dch, 0ebh, 029h, 080h, 07dh
    db 0fch, 020h, 075h, 023h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 090h, 02eh, 0c2h
    db 024h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 003h, 090h, 024h, 0c2h, 024h, 000h, 001h
    db 055h, 0dch, 0e9h, 021h, 0ffh, 0ffh, 0ffh, 0e9h, 01ch, 0ffh, 0ffh, 0ffh, 069h, 045h, 0e4h, 03eh
    db 006h, 000h, 000h, 083h, 0b8h, 018h, 0c2h, 024h, 000h, 001h, 075h, 019h, 08bh, 055h, 0dch, 02bh
    db 055h, 0f4h, 089h, 0d0h, 0c1h, 0fah, 01fh, 02bh, 0c2h, 0d1h, 0f8h, 08bh, 055h, 0f4h, 029h, 0c2h
    db 089h, 055h, 0dch, 0ebh, 026h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 083h, 0b8h, 018h, 0c2h
    db 024h, 000h, 002h, 075h, 010h, 08bh, 045h, 0dch, 02bh, 045h, 0f4h, 08bh, 055h, 0f4h, 029h, 0c2h
    db 089h, 055h, 0dch, 0ebh, 006h, 08bh, 045h, 0f4h, 089h, 045h, 0dch, 0c7h, 045h, 0ech, 000h, 000h
    db 000h, 000h, 08bh, 045h, 0d8h, 0e8h, 0feh, 055h, 001h, 000h, 03bh, 045h, 0ech, 07fh, 00dh, 0e9h
    db 019h, 001h, 000h, 000h, 08bh, 045h, 0ech, 0ffh, 045h, 0ech, 0ebh, 0e6h, 08bh, 045h, 0d8h, 003h
    db 045h, 0ech, 08ah, 000h, 088h, 045h, 0fch, 080h, 07dh, 0fch, 000h, 00fh, 084h, 0fch, 000h, 000h
    db 000h, 069h, 055h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 045h, 0fch, 001h, 0d0h, 080h, 0b8h
    db 012h, 0c2h, 024h, 000h, 001h, 00fh, 085h, 0b4h, 000h, 000h, 000h, 0c7h, 045h, 0f8h, 000h, 000h
    db 000h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 055h, 0fch, 001h, 0d2h, 001h
    db 0d0h, 069h, 055h, 0e4h, 03eh, 006h, 000h, 000h, 066h, 08bh, 080h, 052h, 0c6h, 024h, 000h, 066h
    db 03bh, 082h, 02eh, 0c2h, 024h, 000h, 074h, 032h, 069h, 055h, 0e4h, 03eh, 006h, 000h, 000h, 00fh
    db 0b6h, 045h, 0fch, 001h, 0c0h, 001h, 0c2h, 00fh, 0bfh, 082h, 052h, 0c6h, 024h, 000h, 069h, 055h
    db 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 092h, 02eh, 0c2h, 024h, 000h, 029h, 0c2h, 089h, 0d0h
    db 0c1h, 0fah, 01fh, 02bh, 0c2h, 0d1h, 0f8h, 089h, 045h, 0f8h, 08bh, 045h, 0dch, 003h, 045h, 0f8h
    db 0c1h, 0e0h, 008h, 089h, 045h, 0cch, 08bh, 045h, 0e0h, 0c1h, 0e0h, 008h, 089h, 045h, 0d0h, 031h
    db 0dbh, 08dh, 055h, 0cch, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 04dh, 0fch, 0c1h
    db 0e1h, 002h, 001h, 0c8h, 08bh, 080h, 092h, 0c2h, 024h, 000h, 0e8h, 020h, 03dh, 0ffh, 0ffh, 069h
    db 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 090h, 02eh, 0c2h, 024h, 000h, 069h, 045h, 0e4h
    db 03eh, 006h, 000h, 000h, 003h, 090h, 024h, 0c2h, 024h, 000h, 001h, 055h, 0dch, 0ebh, 029h, 080h
    db 07dh, 0fch, 020h, 075h, 023h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 090h, 02eh
    db 0c2h, 024h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 003h, 090h, 024h, 0c2h, 024h, 000h
    db 001h, 055h, 0dch, 0e9h, 0ech, 0feh, 0ffh, 0ffh, 0e9h, 0e7h, 0feh, 0ffh, 0ffh, 0e9h, 0ffh, 001h
    db 000h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 083h, 0b8h, 018h, 0c2h, 024h, 000h, 000h
    db 00fh, 084h, 004h, 001h, 000h, 000h, 0c7h, 045h, 0ech, 000h, 000h, 000h, 000h, 08bh, 045h, 0d8h
    db 0e8h, 0b3h, 054h, 001h, 000h, 03bh, 045h, 0ech, 07fh, 00dh, 0e9h, 09ch, 000h, 000h, 000h, 08bh
    db 045h, 0ech, 0ffh, 045h, 0ech, 0ebh, 0e6h, 08bh, 045h, 0d8h, 003h, 045h, 0ech, 08ah, 000h, 088h
    db 045h, 0fch, 080h, 07dh, 0fch, 000h, 00fh, 084h, 07fh, 000h, 000h, 000h, 069h, 055h, 0e4h, 03eh
    db 006h, 000h, 000h, 00fh, 0b6h, 045h, 0fch, 001h, 0d0h, 080h, 0b8h, 012h, 0c2h, 024h, 000h, 001h
    db 075h, 028h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 055h, 0fch, 001h, 0d2h, 001h
    db 0d0h, 00fh, 0bfh, 090h, 052h, 0c6h, 024h, 000h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 003h
    db 090h, 024h, 0c2h, 024h, 000h, 001h, 055h, 0dch, 0ebh, 03ch, 080h, 07dh, 0fch, 020h, 075h, 036h
    db 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 080h, 02eh, 0c2h, 024h, 000h, 0c1h, 0e0h
    db 008h, 089h, 045h, 0c8h, 0dbh, 045h, 0c8h, 0dch, 00dh, 090h, 0aeh, 00ah, 000h, 0dch, 00dh, 098h
    db 0aeh, 00ah, 000h, 0e8h, 04bh, 042h, 0ffh, 0ffh, 0dbh, 05dh, 0c8h, 08bh, 045h, 0c8h, 001h, 045h
    db 0dch, 0e9h, 069h, 0ffh, 0ffh, 0ffh, 0e9h, 064h, 0ffh, 0ffh, 0ffh, 069h, 045h, 0e4h, 03eh, 006h
    db 000h, 000h, 083h, 0b8h, 018h, 0c2h, 024h, 000h, 001h, 075h, 019h, 08bh, 055h, 0dch, 02bh, 055h
    db 0f4h, 089h, 0d0h, 0c1h, 0fah, 01fh, 02bh, 0c2h, 0d1h, 0f8h, 08bh, 055h, 0f4h, 029h, 0c2h, 089h
    db 055h, 0dch, 0ebh, 026h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 083h, 0b8h, 018h, 0c2h, 024h
    db 000h, 002h, 075h, 010h, 08bh, 045h, 0dch, 02bh, 045h, 0f4h, 08bh, 055h, 0f4h, 029h, 0c2h, 089h
    db 055h, 0dch, 0ebh, 006h, 08bh, 045h, 0f4h, 089h, 045h, 0dch, 0c7h, 045h, 0ech, 000h, 000h, 000h
    db 000h, 08bh, 045h, 0d8h, 0e8h, 0afh, 053h, 001h, 000h, 03bh, 045h, 0ech, 07fh, 00dh, 0e9h, 0ceh
    db 000h, 000h, 000h, 08bh, 045h, 0ech, 0ffh, 045h, 0ech, 0ebh, 0e6h, 08bh, 045h, 0d8h, 003h, 045h
    db 0ech, 08ah, 000h, 088h, 045h, 0fch, 080h, 07dh, 0fch, 000h, 00fh, 084h, 0b1h, 000h, 000h, 000h
    db 069h, 055h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0b6h, 045h, 0fch, 001h, 0d0h, 080h, 0b8h, 012h
    db 0c2h, 024h, 000h, 001h, 075h, 05ah, 08bh, 045h, 0dch, 0c1h, 0e0h, 008h, 089h, 045h, 0cch, 08bh
    db 045h, 0e0h, 0c1h, 0e0h, 008h, 089h, 045h, 0d0h, 031h, 0dbh, 08dh, 055h, 0cch, 069h, 04dh, 0e4h
    db 03eh, 006h, 000h, 000h, 00fh, 0b6h, 045h, 0fch, 0c1h, 0e0h, 002h, 001h, 0c8h, 08bh, 080h, 092h
    db 0c2h, 024h, 000h, 0e8h, 037h, 03bh, 0ffh, 0ffh, 069h, 055h, 0e4h, 03eh, 006h, 000h, 000h, 00fh
    db 0b6h, 045h, 0fch, 001h, 0c0h, 001h, 0d0h, 00fh, 0bfh, 090h, 052h, 0c6h, 024h, 000h, 069h, 045h
    db 0e4h, 03eh, 006h, 000h, 000h, 003h, 090h, 024h, 0c2h, 024h, 000h, 001h, 055h, 0dch, 0ebh, 03ch
    db 080h, 07dh, 0fch, 020h, 075h, 036h, 069h, 045h, 0e4h, 03eh, 006h, 000h, 000h, 00fh, 0bfh, 080h
    db 02eh, 0c2h, 024h, 000h, 0c1h, 0e0h, 008h, 089h, 045h, 0c8h, 0dbh, 045h, 0c8h, 0dch, 00dh, 0a0h
    db 0aeh, 00ah, 000h, 0dch, 00dh, 0a8h, 0aeh, 00ah, 000h, 0e8h, 015h, 041h, 0ffh, 0ffh, 0dbh, 05dh
    db 0c8h, 08bh, 045h, 0c8h, 001h, 045h, 0dch, 0e9h, 037h, 0ffh, 0ffh, 0ffh, 0e9h, 032h, 0ffh, 0ffh
    db 0ffh, 0c7h, 045h, 0f0h, 001h, 000h, 000h, 000h, 08bh, 045h, 0f0h, 0c9h, 05fh, 05eh, 0c3h
Font_DrawText_ ENDP

end
