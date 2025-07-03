import { useEffect, useState } from 'react';
import { useDebounce } from '@uidotdev/usehooks';
import { RadioBrowserApi, StationSearchType, Station } from 'radio-browser-api';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';
import Stack from '@mui/material/Stack';
import Container from '@mui/material/Container';

import Slider from '@mui/material/Slider';
import VolumeDown from '@mui/icons-material/VolumeDown';

import VolumeUp from '@mui/icons-material/VolumeUp';
import HourglassBottomIcon from '@mui/icons-material/HourglassBottom';
import SearchIcon from '@mui/icons-material/Search';

import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';

import NativeSelect from '@mui/material/NativeSelect';

import TextField from '@mui/material/TextField';

import Fade from '@mui/material/Fade';
import IconButton from '@mui/material/IconButton';
import Snackbar from '@mui/material/Snackbar';
import Box from '@mui/material/Box';
import Alert from '@mui/material/Alert';

import { StationBox } from './StationBox';

const localStorageFavouritesKey = 'favourites';
const uiDebounceTime = 500;
const radioBrowserBaseUrl = 'https://de1.api.radio-browser.info';

let radioBaseUrl: string;
if (process.env.NODE_ENV === 'production') {
  radioBaseUrl = '';
} else {
  radioBaseUrl = 'http://aradio.local';
}

const api = new RadioBrowserApi('ARadio', true);
api.setBaseUrl(radioBrowserBaseUrl);

type Country = { name: string; iso_3166_1: string; stationcount: number };

export function App() {
  const handleChange = () => {};

  const [countries, setCountries] = useState<Country[] | null>(null);
  const [stations, setStations] = useState<Station[] | null>(null);
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [isPlaying, setIsPlaying] = useState<boolean>(false);
  const [currentStationName, setCurrentStationName] = useState<string>('');
  const [currentStationTitle, setCurrentStationTitle] = useState<string>('');
  const [cmdIsLoading, setCmdIsLoading] = useState<boolean>(false);
  const [snackbar, setSnackbar] = useState<{ open: boolean; message: string; isError: boolean}>({
    open: false,
    message: '',
    isError: false,
  });

  const [searchKeyword, setSearchKeyword] = useState<string>('');

  const [volume, setVolume] = useState<number>(0);
  const debouncedVolume = useDebounce(volume, uiDebounceTime);

  const doFetch = async (url: string): Promise<any> => {
    setCmdIsLoading(true);
    try {
      const response = await fetch(radioBaseUrl + url);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const data = await response.text();
      setCmdIsLoading(false);
      return data;
    } catch (error) {
      setCmdIsLoading(false);
      throw error;
    }
  };

  const showMessage = (message: string, isError: boolean = false) => {
    setSnackbar({
      open: true,
      message: message,
      isError: isError,
    });
  };

  const playStream = async (url: string) => {
    try {
      await doFetch(`/play?url=${encodeURIComponent(url)}`);
    } catch (error) {
      showMessage(`Error playing stream`, true);
      console.log('Error playing stream:', error);
    }
    await updateStatus();
  };

  const stopStream = async () => {
    try {
      const response = await doFetch(`/stop`);
    } catch (error) {
      console.log('Error stopping stream:', error);
      showMessage('Stream might be stopped already', true);
    }
    await updateStatus();
  };

  useEffect(() => {
    async function setVolumeAsync() {
      if (debouncedVolume !== 0) {
        const volumeUrl = `/setvolume?value=${debouncedVolume}`;
        try {
          await doFetch(volumeUrl);
        } catch (error) {
          console.log('Error setting volume:', error);
        }
      }
    }
    setVolumeAsync();
  }, [debouncedVolume]);

  async function updateStatus() {
    try {
      const response = await doFetch(`/status`);

      const status = response.split(',');
      const [isPlayingStatus, volumeStatus, stationName, statonTitle] = status;
      setIsPlaying(isPlayingStatus === '1');
      setVolume(parseInt(volumeStatus, 10));
      setCurrentStationName(stationName);
      setCurrentStationTitle(statonTitle);
    } catch (error) {
      showMessage('Error updating status', true);
      console.log('Error updating status:', error);
    }
  }

  useEffect(() => {
    async function fetchInitialData() {
      setCmdIsLoading(true);
      try {
        const apiCountries = await api.getCountries();
        setCountries(apiCountries as Country[]);
      } catch (error) {
        console.log('Error:', error);
        showMessage('Error fetching countries', true);
      }
      await updateStatus();
      setCmdIsLoading(false);
      showFavouriteStations();
    }
    fetchInitialData();
  }, []);

  function sortStations(stations: Station[]): Station[] {
    return stations.sort(
      (a, b) =>
        (b.clickCount ?? 0) +
        (b.votes ?? 0) -
        ((a.clickCount ?? 0) + (a.votes ?? 0))
    );
  }

  async function showStationsByCountry(
    event: React.ChangeEvent<HTMLSelectElement>
  ): Promise<void> {
    setIsLoading(false);
    setStations(null);
    const countryCode = event.target.value;

    try {
      let stationsByCountry = await api.getStationsBy(
        StationSearchType.byCountryCodeExact,
        countryCode
      );
      stationsByCountry = sortStations(stationsByCountry);
      setStations(stationsByCountry);
    } catch (error) {
      console.warn('Error fetching stations by country:', error);
      setSnackbar({
        open: true,
        message: 'Error fetching stations by country',
      });
    }
    setIsLoading(false);
  }

  function getFavouritesFromLocalStorage(): Station[] {
    const favourites = localStorage.getItem(localStorageFavouritesKey);
    return favourites ? JSON.parse(favourites).reverse() : [];
  }

  function isFavourite(id: string) {
    const favourites = getFavouritesFromLocalStorage();
    return favourites.some((fav: Station) => fav.id === id);
  }

  function removeFromFavourites(id: string): void {
    const favourites = getFavouritesFromLocalStorage();
    const updatedFavourites = favourites.filter(
      (fav: Station) => fav.id !== id
    );
    localStorage.setItem(
      localStorageFavouritesKey,
      JSON.stringify(updatedFavourites)
    );

    setStations((prev) =>
      prev
        ? prev.map((station) => (station.id === id ? { ...station } : station))
        : prev
    );
  }
  function addToFavourites(station: Station): void {
    const favourites = getFavouritesFromLocalStorage();
    if (!favourites.some((fav: Station) => fav.id === station.id)) {
      favourites.push(station);
      localStorage.setItem(
        localStorageFavouritesKey,
        JSON.stringify(favourites)
      );

      setStations((prev) =>
        prev ? prev.map((s) => (s.id === station.id ? { ...s } : s)) : prev
      );
    }
  }
  function showFavouriteStations(): void {
    const favourites = getFavouritesFromLocalStorage();
    setStations(favourites);
  }
  async function showStationsByName(showStationsByName: string): Promise<void> {
    if (!showStationsByName || showStationsByName.trim() === '') {
      showMessage('Search term cannot be empty');
      return;
    }

    if (showStationsByName.length < 3) {
      showMessage('Search term must be at least 3 characters long');
      return;
    }

    setIsLoading(true);
    setStations(null);

    try {
      let stationsByName = await api.getStationsBy(
        StationSearchType.byName,
        showStationsByName
      );
      stationsByName = sortStations(stationsByName);
      setStations(stationsByName);
    } catch (error) {
      console.warn('Error fetching stations by name:', error);
      showMessage('Error fetching stations by name', true);
    }
    setIsLoading(false);
  }

  return (
    <Container
      maxWidth='md'
      sx={{
        opacity: cmdIsLoading ? 0.5 : 1,
        pointerEvents: cmdIsLoading ? 'none' : 'auto',
        transition: 'opacity 0.2s',
      }}
    >
      <Stack>
        <Stack
          direction='row'
          spacing={1}
          justifyContent='space-between'
          alignItems='center'
        >
          <Box>
            <Typography variant='h4' component='h4' sx={{ pt: 2 }}>
              📻 ARadio
            </Typography>
          </Box>

          <Box>
            <Button
              variant='contained'
              color={isPlaying ? 'error' : 'primary'}
              onClick={stopStream}
              disabled={cmdIsLoading}
            >
              Stop
            </Button>
          </Box>
        </Stack>
        <Typography variant='subtitle1' sx={{ pt: 2, textAlign: 'center' }}>
          {isPlaying ? currentStationName + ' ' + currentStationTitle : ''}
        </Typography>
        <Stack direction='column' spacing={1}>
          <Stack
            width='100%'
            spacing={2}
            direction='row'
            sx={{ alignItems: 'center', mb: 1 }}
          >
            <VolumeDown />
            <Slider
              aria-label='Volume'
              value={volume}
              min={0}
              max={21}
              onChange={(_, value) =>
                setVolume(typeof value === 'number' ? value : 0)
              }
            />
            <VolumeUp />
          </Stack>
          <FormControl
            fullWidth
            sx={{
              flexDirection: 'row',
              alignItems: 'center',
              gap: 1,
              display: 'flex',
            }}
          >
            <TextField
              value={searchKeyword}
              label='Search by name'
              variant='standard'
              onChange={(e) => setSearchKeyword(e.target.value)}
              onKeyDown={(e) => {
                if (e.key === 'Enter') {
                  showStationsByName(searchKeyword);
                }
              }}
              sx={{ flex: 1 }}
            />
            <IconButton onClick={() => showStationsByName(searchKeyword)}>
              <SearchIcon />
            </IconButton>
          </FormControl>
          <FormControl fullWidth>
            <InputLabel id='search-by-country'></InputLabel>
            <NativeSelect
              defaultValue=''
              variant='filled'
              onChange={showStationsByCountry}
            >
              <option value='' disabled>
                Search by Country
              </option>
              {countries?.map(
                (country: {
                  name: string;
                  iso_3166_1: string;
                  stationcount: number;
                }) => (
                  <option key={country.iso_3166_1} value={country.iso_3166_1}>
                    {country.name} ({country.stationcount})
                  </option>
                )
              )}
            </NativeSelect>
          </FormControl>
          <Button onClick={showFavouriteStations} fullWidth variant='outlined'>
            Favourite Stations
          </Button>
        </Stack>
      </Stack>
      <Stack padding={1}>
        {!isLoading &&
          (stations && stations.length > 0 ? (
            <Stack spacing={1}>
              {stations.map((station) => (
                <StationBox
                  key={station.id}
                  isFavourite={isFavourite(station.id)}
                  station={station}
                  onAddFavorites={addToFavourites}
                  onRemoveFavourites={removeFromFavourites}
                  onPlayStreamURL={playStream}
                />
              ))}
            </Stack>
          ) : (
            <Typography variant='body2' color='text.secondary'>
              Nothing stations found.
            </Typography>
          ))}

        {isLoading && (
          <Typography variant='body2' color='text.secondary' align='center'>
            <Fade in={isLoading} timeout={200}>
              <span>
                <HourglassBottomIcon fontSize='small' /> Wait...
              </span>
            </Fade>
          </Typography>
        )}
      </Stack>

      <Snackbar
        open={snackbar.open}
        autoHideDuration={4000}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'center' }}
        onClose={() => setSnackbar({ open: false, message: '', isError: false })}
       
      >
        <Alert
          severity={snackbar.isError ? 'error' : 'info'}
          variant='filled'
          sx={{ width: '100%' }}
        >
          {snackbar.message}
        </Alert>
      </Snackbar>
    </Container>
  );
}
