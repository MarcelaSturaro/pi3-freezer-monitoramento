import {useEffect, useState} from 'react';
import {Line} from 'react-chartjs-2';
import axios from 'axios';
import 'chart.js/auto';

function App() {
  const [dados, setDados] = useState([]);

  useEffect (() => {
    const fetchData = async () => {
      try {
	const response = await axios.get('https://pi3-freezer-monitoramento.onrender.com/api/temperatura');
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
    labels: dados.map(d => new Date(d._time).toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'})),
    datasets: [{
	label: "Temperatura do Freezer (°C)",
	data: dados.map(d => d._value),
 	borderColor: 'rgb(75, 192, 192)',
        backgroundColor: 'rgba(75, 192, 192, 0.2)',
	fill: true,
      }],
    };

    const options = {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
	tooltip: {enabled: true},
	legend: {position:'top'},
      },
      scales: {
	y: {title: {display: true, text: 'Temperatura (°C)'}, beginAtZero: false},
	x: {title: {display: true, text: 'Horário'}, ticks: {autoSkip: true, maxTicksLimit:10} },
      },
    };

    return(
      <div style={{maxWidth: '1000px', margin: '0 auto', padding: '20px', fontFamily: 'Sego e UI, Arial' }}>
	<h1 style={{textAlign: 'center', color: '#2c3e50'}}> Monitoramento de Freezer - Mercado Mamma Mia</h1>
	<div style={{background: 'white', borderRadius: '12px', padding: '20px', boxShadow: '0 2px 10px rgba(0, 0, 0, 0.1)', height: '450px' }}>
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


















 