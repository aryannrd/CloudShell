import './App.css';
import 'xterm/css/xterm.css';
import { Terminal } from 'xterm';
import { useEffect } from 'react';

function App() {
    const socket = new WebSocket(
        "ws://localhost:8000/terminal"
    );

    useEffect(() => {
        const term = new Terminal();
        term.open(document.getElementById("terminal"));
        term.write(
            'Hello from \x1B[1;3;31mxterm.js\x1B[0m $ '
        );
        term.onData((data) => {
            term.write(data);
        });
        term.onData((data)=>{
            socket.send(data);
        });
        socket.onmessage = (event)=>{
            term.write(event.data);
        };
        return () => {
            term.dispose();
        };
    }, []);
    return (
        <div className="App">
            <h1>Shell Dashboard</h1>
            <div id="terminal"></div>
        </div>
    );
}
export default App;