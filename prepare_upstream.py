from pathlib import Path
import subprocess, shutil
root = Path(__file__).resolve().parent
dest = root/'optiscaler-nr-reference'
if dest.exists():
    raise SystemExit('Destination already exists; use a fresh checkout to avoid overwriting work.')
def run(*args): subprocess.run(args, cwd=dest, check=True)
subprocess.run(['git','clone','https://github.com/Dagherbou/OptiScaler_DLSSNR.git',str(dest)], check=True)
run('git','checkout','--detach','433cc11d8a6b92dfe4977de4dd88ffe0afbec781')
run('git','submodule','update','--init','--recursive')
run('git','apply','--check',str(root/'patches/optiscaler.patch'))
run('git','apply',str(root/'patches/optiscaler.patch'))
for path in (root/'optiscaler-extras').iterdir(): shutil.copy2(path,dest/path.name)
print('Pinned upstream and NR Studio changes prepared:',dest)
