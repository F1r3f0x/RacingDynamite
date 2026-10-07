"""Run the actual menu intro lifetime block as C89 with deterministic inputs."""
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class MenuStartupTests(unittest.TestCase):
    def test_intro_decode_window_and_release(self):
        compiler = shutil.which('gcc')
        self.assertIsNotNone(compiler)
        source = (ROOT / 'decomp/src/menu.c').read_text()
        start = source.index('    /* 3. Handle intro skip */')
        end = source.index('    /* 6. Authentic intro palette fades */', start)
        block = source[start:end]
        harness = r'''
#include <assert.h>
#include <stddef.h>
#include <string.h>
typedef unsigned char uint8_t;
typedef struct { uint8_t *file_data; int current_frame; uint8_t *palette; } CdpFile;
static CdpFile g_MenuCdp;
static double g_MenuTimeSeconds, g_MenuCdpFrameAccum;
static int g_MenuCdpPendingLoad, g_MenuCdpActiveIndex;
static uint8_t *g_pMenuCdpFiles[6];
static uint8_t files[6];
static int decoded, released, initialized, result_code, palette_updates;
static void VGA_SetPaletteRaw(const uint8_t *palette) {
    assert(palette == g_MenuCdp.palette); palette_updates++;
}
static int Input_WasKeyPressed(int key) { (void)key; return 0; }
static void Mem_Free(int flags, void *ptr) {
    (void)flags; assert(ptr != NULL); released++;
}
static void Menu_InitCarViewport(void) { initialized++; }
static int Cdp_DecodeFrame(CdpFile *cdp) {
    assert(released == 0); assert(cdp->file_data != NULL);
    decoded++; cdp->current_frame++; return result_code;
}
static int Cdp_OpenFile(CdpFile *cdp) {
    assert(released == 0); assert(cdp->file_data != NULL); cdp->current_frame=0; return 1;
}
static void tick(double delta, int key) {
'''
        checks = r'''
}
static void reset(double time) {
    int i;
    for (i=0;i<6;i++) g_pMenuCdpFiles[i]=&files[i];
    g_MenuCdp.file_data=&files[0]; g_MenuCdp.current_frame=0;
    g_MenuCdp.palette=&files[0]; palette_updates=0;
    g_MenuTimeSeconds=time; g_MenuCdpFrameAccum=0.0;
    g_MenuCdpPendingLoad=1; g_MenuCdpActiveIndex=0;
    decoded=released=initialized=0; result_code=1;
}
int main(void) {
    reset(28.8); tick(10.0,0);
    assert(decoded==0 && g_MenuCdpFrameAccum==0.0);
    reset(29.0); tick(10.0,0);
    assert(decoded==1 && g_MenuCdpFrameAccum<2.393103448275862);
    tick(3.0,0); assert(palette_updates==1);
    tick(3.0,0); assert(palette_updates==1);
    reset(1278.0); tick(10.0,0); tick(10.0,0);
    assert(decoded==0 && released==6 && initialized==1);
    reset(100.0); tick(10.0,13); tick(10.0,0);
    assert(decoded==0 && released==6 && initialized==1);
    reset(100.0); result_code=0; tick(3.0,0);
    assert(decoded==1 && g_MenuCdpActiveIndex==1);
    assert(g_MenuCdp.file_data==&files[1]);
    reset(1277.0); g_MenuCdpActiveIndex=5; result_code=0;
    tick(3.0,0); assert(g_MenuTimeSeconds==1278.0);
    tick(3.0,0); assert(decoded==1 && released==6 && initialized==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'menu_test.c'
            exe = Path(tmp) / 'menu_test.exe'
            path.write_text(harness + block + checks)
            compiled = subprocess.run([compiler, '-std=c89', '-pedantic', '-Wall', '-Wextra', '-Werror', str(path), '-o', str(exe)], capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            result = subprocess.run([str(exe)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

if __name__ == '__main__':
    unittest.main()
