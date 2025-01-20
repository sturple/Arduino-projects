# Arduino Projects

## CircuitPython

### Card Reader
#### venv create
```python3 -m venv venv-card-reader```

#### Install dependencies
```bash
cd ./circuitpython/card_reader/
python3 source venv-cr/bin/activate
pip3 install -r requirements.txt
# one terminal run
textual console -x SYSTEM -x EVENT -x DEBUG -x INFO
# another terminal run
textual run --dev reader.py
```

#### Resources
[textual dev tools](https://textual.textualize.io/guide/devtools/)

