import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';
import Stack from '@mui/material/Stack';
import Container from '@mui/material/Container';
import Paper from '@mui/material/Paper';
import { useEffect } from 'react';
import { RadioBrowserApi, StationSearchType } from 'radio-browser-api';
import Slider from '@mui/material/Slider';
import VolumeDown from '@mui/icons-material/VolumeDown';
import VolumeUp from '@mui/icons-material/VolumeUp';

const api = new RadioBrowserApi('ARadio', true);
api.setBaseUrl('https://de2.api.radio-browser.info');

// Favouries
// By Countru
// By Search box name


export function App() {
  const handleChange = () => {};
  useEffect(() => {
    async function fetchInitialData() {
      try {
        // countries / status
        //const countries = await api.getCountries();
        //const stations = await api.getStationsBy(StationSearchType.byCountryCodeExact, 'RO');
        //const stations = await api.getStationsBy(
        //  StationSearchType.byName,
        //  'iCat'
        //);
        //console.log(stations, countries, api.getBaseUrl());
        // get status
      } catch (error) {
        console.warn('Error:', error);
      }
    }
    fetchInitialData();
  }, []);
  return (
      <Container maxWidth='sm'>
        <Paper
          elevation={3}
          sx={{
            background: { xs: 'none', sm: 'initial' },
            boxShadow: { xs: 'none', sm: 3 },
            p: 0,
          }}
        >
        <Stack
          padding={1} 
        >
          <Typography variant='h4' component='h4' >
            ARadiox
          </Typography>

          <Stack
            width='100%'
            spacing={2}
            direction='row'
            sx={{ alignItems: 'center', mb: 1 }}
          >
            <VolumeDown />
            <Slider
              aria-label='Volume'
              value={10}
              min={0}
              max={21}
              onChange={handleChange}
            />
             <VolumeUp />
          </Stack>
          <Stack direction='row' spacing={1}>
            <Button variant='contained'>Favourites</Button>
          </Stack>
        </Stack>
        </Paper>
      </Container>

  );
}
