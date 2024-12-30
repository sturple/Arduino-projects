"""
Code browser example.

Run with:

    python code_browser.py PATH
"""

from __future__ import annotations
# For Interface
import sys



# For application

from threading import Thread
from pn532 import PN532
import time
from rich.syntax import Syntax
from rich.traceback import Traceback
from textual import events, on
from textual.app import App, ComposeResult
from textual.binding import Binding
from textual.containers import Container, VerticalScroll
from textual.reactive import reactive, var
from textual.widgets import (
    Button,
    ContentSwitcher,
    DataTable,
    Footer,
    Header,
    Input,
    Label,
    ListItem,
    ListView,
    MarkdownViewer,
    OptionList,
    ProgressBar,
    RadioSet,
    RichLog,
    Select,
    SelectionList,
    Switch,
    TextArea,

    Log,
)


            


class CardReader(App):
    """Textual code browser app."""
    _log = None
    _pn532 = None
    _loop = None
    _loop_event = None
    _uid = None


    CSS_PATH = "reader.tcss"
    BINDINGS = [
        Binding(
            "ctrl+e",
            "edit_card_info",
            "Edit Card Information",
            tooltip="Edit Card Information",
        ),
        Binding(
            "ctrl+r",
            "read_card_info",
            "Read Card Information",
            tooltip="Read Card Information",
        ),
        Binding(
            "ctrl+m",
            "edit_card_data",
            "Edit Card Data",
            tooltip="Edit Card Data",
        ),
        Binding(
            "ctrl+d",
            "read_card_data",
            "Read Card Data",
            tooltip="Read Card Data",
        )
    ]

    def print_hex_data(self, data)->None:
        try:
            self._log.write_line(" ")
            for i in data:
                self._log.write(hex(i) )
                self._log.write(' ')
            self._log.write_line(" ")
        except:
            """exception"""
            self._log.write_line("Error with the byte data")


    async def read_card_block(self, current_block) -> None:
        """Read the current block"""
        return self._pn532.read_card_block(current_block)


    async def read_card(self) -> None:
        current_block = 5
        self._uid = self._pn532.read_card()
        self.print_hex_data(self._uid)
        if self._uid is not None:
            auth = self._pn532.authenticate_card_block(self._uid, current_block)
            if auth:
                self._log.write_line("Authenticated")
                data = await self.read_card_block(current_block)
                self.print_hex_data(data)
            else:
                self._log.write_line("Not Authenticated")


    @on(Button.Pressed, "#read_card_info")
    async def action_read_card_info(self) -> None:
        self._log.write_line("Please scan your RFID/NFC card...")
        self.run_worker(self.read_card(), exclusive=True, thread=True)


    @on(Button.Pressed, "#edit_card_info")
    def action_edit_card_info(self) -> None:
        self._log.write_line('edit card info')


    @on(Button.Pressed, "#read_card_data")
    def action_read_card_data(self) -> None:
        self._log.write_line('read card data')
        self.query_one(ContentSwitcher).current = 'markdown'

    @on(Button.Pressed, "#edit_card_data")
    def action_edit_card_data(self) -> None:
        self._log.write_line('edit card data')
        self.query_one(ContentSwitcher).current = 'data-table'

    def on_ready(self) -> None:
        self._log = self.query_one(Log)
        self._pn532 = PN532()
        ic, ver, rev, support = self._pn532.get_version()
        self._log.write_line("Found PN532 with firmware version:{0}.{1} ID: {2} Support: {3} ".format(ver, rev, ic, support))


    def compose(self) -> ComposeResult:
        """Compose our UI."""
        yield Header()
        yield Container(
            Container(
                Button("Edit Card Information", id="edit_card_info", variant="primary"),
                Button("Read Card Information", id="read_card_info", variant="primary"),
                Button("Edit Card Data", id="edit_card_data", variant="primary"),
                Button("Read Card Data", id="read_card_data", variant="primary"),
                id="button-panel",
            ),
            Log(highlight=True, id="Logger"),
            id="control-panel")
        with ContentSwitcher(initial="data-table", id="data-panel"):  
            yield VerticalScroll(
                Button("data-table",variant="primary"),
                id="data-table")
            yield VerticalScroll(
                Button("markdown",variant="primary"),
                id="markdown")
        yield Footer()

    def on_mount(self) -> None:
        """on mount"""

class ThreadWithReturnValue(Thread):
    
    def __init__(self, group=None, target=None, name=None,
                 args=(), kwargs={}, Verbose=None):
        Thread.__init__(self, group, target, name, args, kwargs)
        self._return = None

    def run(self):
        if self._target is not None:
            self._return = self._target(*self._args,
                                                **self._kwargs)
    def join(self, *args):
        Thread.join(self, *args)
        return self._return


class BaseThread(Thread):
    def __init__(self, callback=None, callback_args=None, *args, **kwargs):
        target = kwargs.pop('target')
        super(BaseThread, self).__init__(target=self.target_with_callback, *args, **kwargs)
        self.callback = callback
        self.method = target
        self.callback_args = callback_args

    def target_with_callback(self):
        self.method()
        if self.callback is not None:
            self.callback(*self.callback_args)


if __name__ == "__main__":
    CardReader().run()
    
    

