from __future__ import annotations
# For application
from pn532 import PN532
from textual.timer import Timer
from textual import events, on
from textual.app import App, ComposeResult
from textual.binding import Binding
from textual.reactive import reactive
from textual.containers import Container, VerticalScroll, Center, Middle
from textual.widget import Widget
from textual.widgets import (
    Button,
    ContentSwitcher,
    DataTable,
    Footer,
    Header,
    ProgressBar,
    Input,
    Label,
    Select
)


class CardReader(Widget):
    """Textual code browser app."""
    _log = None
    _pn532 = None
    _loop = None
    _loop_event = None
    _uid = None

    CSS_PATH = "reader.tcss"
    BINDINGS = [
        Binding(
            # "ctrl+r",
            "r",
            "read_all_card_data",
            "Read Card Data",
            tooltip="Read Card Data",
        ),
        Binding(
            "e",
            "edit_card_data",
            "Edit Card Data",
            tooltip="Edit Card Data",
        ),
    ]


    # Utility function which creates a tuple of the hex data / byte data from the bytearray from the Mifare card
    def get_hex_byte_data(self, data) -> tuple:
        """this gets lists of both the bytes, and the ascii"""
        hex_data = []
        byte_data = []
        for i in data:
            hex_data.append(hex(i))
            mychar = '.' if i == 0 else chr(i)
            byte_data.append(mychar)

        return (hex_data, byte_data)

    def create_row(self, current_sector, current_block, table, data):
        """creates a row in datatable"""
        hex_data, byte_data = self.get_hex_byte_data(data)
        block = str(current_block)
        text_column = ''.join(byte_data)
        text_column.replace('', '.')
        table.add_row(str(current_sector), str(block), text_column, *hex_data)

    def read_card_sector(self, current_sector=1) -> list:
        self._uid = self._pn532.read_card()
        if self._uid is not None:
            return self._pn532.read_card_sector(self._uid, current_sector)
            

    async def read_all_card_sectors(self, table) -> None:
        """all sectors"""
        self.query_one(ProgressBar).update(total=16, progress=0)
        for x in list(range(0, 16)):
            data = self.read_card_sector(x)
            self.create_row(x, 0, table, data[2][0])
            self.create_row(x, 1, table, data[2][1])
            self.create_row(x, 2, table, data[2][2])
            self.create_row(x, 'trl', table, data[1])
            self.query_one(ProgressBar).advance(1)

    # Action call back from the 'r' key shortcut to read.
    async def action_read_all_card_data(self) -> None:
        self.query_one(ContentSwitcher).current = 'read-card-all'
        self.notify("Please scan your RFID/NFC card...", title="Scan Card")
        table = self.query_one(DataTable)
        table.focus()
        table.clear(columns=True)
        table.add_columns("Sec", "Blk", "Text", "B0", "B1", "B2", "B3", "B4",
                          "B5", "B6", "B7", "B8", "B9", "B10", "B11", "B12", "B13", "B14", "B15")
        self.run_worker(self.read_all_card_sectors(
            table), exclusive=True, thread=True)


    # Action call back from the 'e' key shortcut to edit
    def action_edit_card_data(self) -> None:
        """binding to edit card data"""
        self.query_one(ContentSwitcher).current = 'edit-card'

    # Button submit for the update Mifare card data.
    @on(Button.Pressed, "#write-card-data")
    def action_write_card_data(self) -> None:
        """submits card data"""
        print('button pressed')

    # Select change to get the new data for editing.
    @on(Select.Changed, "#write-card-select")
    def select_changed(self, event: Select.Changed) -> None:
        print(event.value)
        # self.run_worker(self.read_all_card_sectors(table), exclusive=True, thread=True)
        self.title = str(event.value)

    # Textual Widget inherit function, which composes the UI.
    def compose(self) -> ComposeResult:
        """Compose our UI."""
        with ContentSwitcher(initial="edit-card"):
            yield VerticalScroll(
                ProgressBar(total=16, show_eta=False),
                DataTable(id="card-data-table"),
                id="read-card-all"
            )
            yield VerticalScroll(
                Select.from_values(list(range(0, 16)), id="write-card-select"),
                Button("Submit", id="write-card-data"),
                id="edit-card"
            )

    # Textual Widget inherit function, which runs when the widget is mounted.
    def on_mount(self) -> None:
        """on mount"""
        self._pn532 = PN532()
        ic, ver, rev, support = self._pn532.get_version()
        print("Found PN532 with firmware version:{0}.{1} ID: {2} Support: {3} ".format(
            ver, rev, ic, support))


class CardReaderApp(App):
    def compose(self) -> ComposeResult:
        yield Header()
        yield CardReader()
        yield Footer()


if __name__ == "__main__":
    app = CardReaderApp()
    app.run()
