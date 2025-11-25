# Build and Install

Install poetry first.


```shell
pip3 install poetry
```

add poetry to env PATH as upper install cmd indicated

build wheel package

```shell
make install
```

pip3 install dist/flight_profiler-1.0.0b1-cp39-cp39-macosx_12_0_arm64.whl

# Usage

```shell
flight_profiler py_pid
```

# Test
run all unit test with command

```shell
make test
```

# Plugin Development
If you want to provide a command, for example: Command "test" and output som message in streaming

## Server Plugin
Firstly you should develop a server plugin.It should be located in path: src/python/plugins, with name style server_plugin_${command}.py.

Define get_instance function and return your plugin instance. The Plugin Instance should implements do_action. It's a async function, you can output your stream message via asyncio.Queue.

```python
from asyncio import sleep

from flight_profiler.plugins.server_plugin import Message, ServerPlugin, ServerQueue


class TestServerPlugin(ServerPlugin):
    def __init__(self, cmd: str, out_q: ServerQueue):
        super().__init__(cmd, out_q)

    async def do_action(self, param):
        print("server plugin test do action")
        i = 0
        while i < 5:
            i = i + 1
            # stream message 1-5, message 5 marked as end
            await self.out_q.output_msg(
                Message(True if i == 5 else False, "message-" + str(i))
            )
            await sleep(1)


def get_instance(cmd: str, out_q: ServerQueue):
    return TestServerPlugin(cmd, out_q)

```



## Cli Plugin
Then develop a cli plugin. It should be located in path: src/python/plugins, with name style cli_plugin_${command}.py.

Define get_instance function and return your plugin instance. The Plugin Instance should implements do_action. It's a sync function, you can put request to http server and iterate stream message and print to Console.

```python
from flight_profiler.plugins.cli_plugin import BaseCliPlugin
from flight_profiler.utils.cli_util import common_plugin_execute_routine


class TestCliPlugin(BaseCliPlugin):
    def __init__(self, port, server_pid):
        super().__init__(port, server_pid)

    def do_action(self, cmd):
        common_plugin_execute_routine(
            cmd="test",
            param=cmd,
            port=self.port,
            raw_text=True
        )

    def on_interrupted(self):
        pass


def get_instance(port: str, server_pid: int):
    return TestCliPlugin(port, server_pid)

```

Command effect like:
![img.png](docs/images/test_cli.png)
