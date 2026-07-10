from maya import mel

import os
import shlex
import subprocess
import platform

def sanitizeAndRun(commands):
    """
    Run a command, or series of commands, in a sanitized environment free from
    paths to Maya. Environment variables that start with "MAYA_" are not
    sanitized.
    """
    def scrubMayaFromEnv(env_key, env_value):
        values = env_value.split(os.pathsep)

        maya_loc = os.environ['MAYA_LOCATION']

        if not env_key.lower().startswith('maya_'):
            values = [value for value in values if maya_loc not in value and
                                                'autodesk' not in value.lower() and
                                                'mayausd'  not in value.lower()]

        return os.pathsep.join(values)

    hfs_path = mel.eval('houdiniEngine_getHfsPath(false)')

    if not hfs_path:
        return False

    env = os.environ.copy()
    env['HFS'] = hfs_path

    for k, v in env.items():
        env[k] = scrubMayaFromEnv(k, v) if not k.lower().startswith('maya_') else v

    env['PATH'] = os.path.join(hfs_path, 'bin') + os.pathsep + env['PATH']

    DEBUG = False

    for command in commands:
        cmd_args = shlex.split(command)

        if DEBUG:   # noqa
            p = subprocess.Popen(cmd_args, env=env,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = p.communicate()

            if p.returncode != 0:
                print(stdout)
                print(stderr)
                return False
        else:
            subprocess.Popen(cmd_args, env=env,
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    return True
