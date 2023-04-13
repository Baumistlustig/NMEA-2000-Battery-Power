import express from 'express';
import cors from 'cors';
import fs from 'fs';
import bodyParser from 'body-parser';

const app = express();

app.use(cors());
app.set('view engine', 'ejs');

app.get('/', (req, res) => {
    console.log('GET /', req.body, req.query, req.params, req.headers);

    fs.readFile('views/data.json', 'utf8', (err, data) => {
        if (err) {
            res.send(err);
            return;
        }

        data = JSON.parse(data);

        res.render('index', { voltage: ( data.voltage / 20.075888237 ) });
    });
});

app.post('/', bodyParser.json(), (req, res) => {
    console.log('POST /', req.body, req.query, req.params, req.headers);
    fs.readFile('views/data.json', 'utf8', (err, data) => {
        if (err) {
            res.send(err);
            return;
        }

        data = JSON.parse(data);
        

        const payload = JSON.stringify({
            voltage: req.body['voltage'] ? req.body['voltage'] : data.voltage,
            turn: req.body['turn'] !== null  ? req.body['turn'] : data.turn,
        });
        

        fs.writeFile('views/data.json', payload, (err) => {
            if (err) {
                res.send(err);
                return;
            }
        });

        res.send(data.turn);
    });
});

app.listen(3000, () => {
    console.log('Server started on port 3000');
});