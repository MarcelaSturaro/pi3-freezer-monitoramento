import {useEffect, useState} from 'react';
import {Line} from 'react-chartjs-2';
import axios from 'axios';
import 'chart.js/auto';

function App() {
  const [dados, setDados] = useState([]);

  useEffect (() => {
    const fetchData = async () => {
      try {
	const response = await axios.get('http://localhost:3001/api/temperatura');
	setDados(response.data);
      } catch (error) {
	console.error('Erro ao buscar dados: ', error);
      }
    };
    fetchData();
    const interval = setInterval(fetchData, 30000);
    return () => clearInterval(interval);
  }, []);


  const chartData = {
    labels: dados.map(d => new Date(d._time).toLocaleTimeString()),
    datasets: [{
	label: "Temperatura do Freezer (°C)",
	data: dados.map(d => d._value),
	borderColor: 'rgba(75, 192, 192, 0.2)',
	fill: true,
      }],
    };

    const options = {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
	y: {title: {display: true, text: 'Temperatura (°C)'} },
	x: {title: {display: true, text: 'Horário'} },
      },
    };

    return(
      <div style={{padding: '20px', fontFamily: 'Arial' }}>
	<h1>Monitoramento de Freezer - Mercado Mamma Mia</h1>
	<div style={{ height: '400px', marginTop: '20px' }}>
	  <Line data={chartData} options={options} />
	</div>
	<div vw="true" className="enabled">
	  <div vw-access-button="true" className="active"></div>
	  <div vw-plugin-wrapper="true">
	    <div className="vw-plugin-top-wrapper"></div>
	  </div>
	</div>
      </div>
    );
  }
  

  export default App;


















 