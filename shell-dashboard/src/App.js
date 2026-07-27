import './App.css';
import 'xterm/css/xterm.css';
import { FitAddon } from '@xterm/addon-fit';
import { Terminal } from 'xterm';
import { useEffect, useRef } from 'react';

function App() {
    const terminalRef = useRef(null);
    const termInstanceRef = useRef(null);

    useEffect(() => {
        let reconnectDelay = 1000;
        if (termInstanceRef.current || !terminalRef.current) return;
        const term = new Terminal({
            cursorBlink: true,
            convertEol: true,
            fontSize: 14,
            fontFamily: '"JetBrains Mono", monospace',
            fontWeight: 'normal',
            letterSpacing: 0,
            lineHeight: 1.2,
            theme: {
                background: '#010409',
                foreground: '#e6edf3',
                cursor: '#58a6ff',
                selectionBackground: 'rgba(88, 166, 255, 0.3)',
                black: '#0d1117',
                red: '#ff7b72',
                green: '#7ee787',
                yellow: '#d2a8ff',
                blue: '#58a6ff',
                magenta: '#bc8cff',
                cyan: '#39c5cf',
                white: '#ffffff'
            }
        });

        window._term = term;
        termInstanceRef.current = term;
        const fitAddon = new FitAddon();
        term.loadAddon(fitAddon);
        term.open(terminalRef.current);
        term.focus();
        terminalRef.current.addEventListener("click", () => term.focus());
        setTimeout(() => fitAddon.fit(), 100);

        let socket;
        let handleResize;
        function connect() {
            socket = new WebSocket("wss://cloudshell-86nr.onrender.com/terminal");
            window._socket = socket;
            socket.onmessage = (event) => {
                term.write(event.data);
            };
            socket.onclose = () => {
                term.write("\r\n\x1b[33mSession ended. Reconnecting...\x1b[0m\r\n");
                setTimeout(() => {
                    reconnectDelay = Math.min(reconnectDelay * 2, 30000);
                    connect();
                }, reconnectDelay);
            };

            socket.onopen = () => {
                reconnectDelay = 1000;
                socket.send(JSON.stringify({ type: "resize", cols: term.cols, rows: term.rows }));
            };
            handleResize = () => {
                fitAddon.fit();
                if (socket.readyState === WebSocket.OPEN) {
                    socket.send(JSON.stringify({ type: "resize", cols: term.cols, rows: term.rows }));
                }
            };
            window.addEventListener("resize", handleResize);
        }

        connect();
        term.onData((data) => {
            if (socket && socket.readyState === WebSocket.OPEN) {
                socket.send(data);
            }
        });

        return () => {
            if (handleResize) window.removeEventListener("resize", handleResize);
            if (socket) socket.close();
            term.dispose();
            termInstanceRef.current = null;
        };
    }, []);

    return (
        <div className="App">
            <h1 className="title1">CloudShell Dashboard</h1>
            <p className="description">
                A Unix shell built from scratch in C, with a full-stack telemetry and prediction pipeline.
                CloudShell executes real system commands, tracks usage patterns, and uses a Markov chain
                model to predict the next command. All of this is served through a Dockerized FastAPI
                backend and a React dashboard. Deployed on Render and Vercel.
            </p>
            <div className="terminal-wrapper">
                <div style={{ padding: "15px", width: "100%", height: "100%" }}>
                    <div ref={terminalRef} id="terminal"></div>
                </div>
            </div>
            <div className="stack">
                <span>C</span>
                <span>FastAPI</span>
                <span>SQLite</span>
                <span>Docker</span>
                <span>React</span>
                <span>xterm.js</span>
                <span>Render</span>
                <span>Vercel</span>
            </div>
        </div>
    );
}
export default App