import './App.css';
import 'xterm/css/xterm.css';
import { FitAddon } from '@xterm/addon-fit';
import { Terminal } from 'xterm';
import { useEffect, useRef } from 'react';

function App() {
    const terminalRef = useRef(null);
    const termInstanceRef = useRef(null);

    useEffect(() => {
        if (termInstanceRef.current || !terminalRef.current) return;
        const term = new Terminal({ cursorBlink: true, convertEol: true});
        const fitAddon = new FitAddon();
        termInstanceRef.current = term;
        term.loadAddon(fitAddon);
        term.open(terminalRef.current);
        setTimeout(() => fitAddon.fit(), 100);

        const socket = new WebSocket("ws://localhost:8000/terminal");
        socket.onopen = () => {
            socket.send(JSON.stringify({ type: "resize", cols: term.cols, rows: term.rows }));
            term.onData((data) => {
                socket.send(data);
            });
        };
        socket.onmessage = (event) => {
            term.write(event.data);
        };
        const handleResize = () => {
            fitAddon.fit();
            if (socket.readyState === WebSocket.OPEN) {
                socket.send(JSON.stringify({ type: "resize", cols: term.cols, rows: term.rows }));
            }
        };
        window.addEventListener("resize", handleResize);

        return () => {
            window.removeEventListener("resize", handleResize);
            socket.close();
            term.dispose();
            termInstanceRef.current = null;
        };
    }, []);

    return (
        <div className="App">
            <h1>Shell Dashboard</h1>
            <div ref={terminalRef} id="terminal"></div>
        </div>
    );
}
export default App