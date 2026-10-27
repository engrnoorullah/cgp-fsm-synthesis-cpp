import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
BIN = ROOT / 'build/cgp'

class CLI(unittest.TestCase):
    def run_cli(self, *args):
        return subprocess.run([str(BIN), *map(str,args)], cwd=ROOT, capture_output=True, text=True, timeout=20)

    def test_roundtrip_and_all_xor_rows(self):
        with tempfile.TemporaryDirectory() as tmp:
            model = pathlib.Path(tmp)/'xor.cgp'
            self.assertEqual(self.run_cli('train','examples/xor.table',model,'--seed',10,'--generations',10000).returncode,0)
            for bits,want in [('00','0'),('01','1'),('10','1'),('11','0')]:
                result=self.run_cli('eval',model,bits)
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertEqual(result.stdout.strip(),want)

    def test_bounded_search_and_invalid_arguments(self):
        with tempfile.TemporaryDirectory() as tmp:
            model=pathlib.Path(tmp)/'model.cgp'
            result=self.run_cli('train','examples/xor.table',model,'--generations',0)
            self.assertIn(result.returncode,[0,2])
            self.assertTrue(model.exists())
            for args in [('--nodes','0'),('--nodes','513'),('--seed','-1'),('--offspring','0'),('--unknown','1')]:
                self.assertEqual(self.run_cli('train','examples/xor.table',model,*args).returncode,1)
            bad=pathlib.Path(tmp)/'bad.cgp'
            bad.write_text('CGP1 1 1 1\n0 1 0\n1\n')
            self.assertEqual(self.run_cli('eval',bad,'0').returncode,1)

if __name__=='__main__': unittest.main()
