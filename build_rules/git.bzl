gitclone = """
set -e
dir=$(basename $PWD)
cd ../
rm -rf $dir
git clone --depth=1 {remote} $dir
cd $dir
if [ -n "{commit}" ]; then
    git reset --hard {commit}
fi
"""

def _impl(ctx):
    bash_exe = ctx.os.environ["BAZEL_SH"] if "BAZEL_SH" in ctx.os.environ else "bash"
    ctx.execute([
        bash_exe,
        '-c',
        gitclone.format(remote=ctx.attr.remote, commit=ctx.attr.commit)
    ])
    return

git_repo = repository_rule(
    implementation = _impl,
    attrs={
        "remote": attr.string(
            mandatory=True,
        ),
        "commit": attr.string(
        ),
    },
)
