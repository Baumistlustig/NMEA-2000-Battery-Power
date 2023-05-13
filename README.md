# boat controll

This is a project, which reads the voltage of a battery, and posts it to a influxDB which displays the data in a grafana dashboard. If the voltage is lower than 12.5V, it automatically turns the main relay off to prevent a drain of the battery.
