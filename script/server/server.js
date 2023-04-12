import express from 'express';
import cors from 'cors';
import fs from 'fs';
import bodyParser from 'body-parser';

const app = express();

app.use(cors());
app.set('view engine', 'ejs');

app.get('/', (req, res) => {
    fs.readFile('views/data.json', 'utf8', (err, data) => {
        if (err) {
            res.send(err);
            return;
        }
        
        data = JSON.parse(data);

        res.render('index', { voltage: data.voltage });
    });
});

app.listen(3000, () => {
    console.log('Server started on port 3000');
});