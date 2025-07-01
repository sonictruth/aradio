import StarOutlineIcon from '@mui/icons-material/StarOutline';
import Star from '@mui/icons-material/Star';

import Paper from '@mui/material/Paper';
import IconButton from '@mui/material/IconButton';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';
import { Station } from 'radio-browser-api';

import flags from './flags';

function getFlag(countryCode: string): React.ReactNode {
  if (!countryCode) return '';
  const flagObj = flags.find(
    (f) => f.code.toUpperCase() === countryCode.toUpperCase()
  );
  return flagObj ? flagObj.emoji : countryCode;
}

export function StationBox({
  station,
  isFavourite,
  onAddFavorites,
  onRemoveFavourites,
  onPlayStreamURL,
}: {
  station: Station;
  isFavourite: boolean;
  onAddFavorites: (station: Station) => void;
  onRemoveFavourites: (stationId: string) => void;
  onPlayStreamURL: (url: string) => void;
}) {


  return (
    <Paper key={station.id} sx={{ p: 1 }}>
      <Typography variant='subtitle1'>
        {station.favicon && (
          <img
            src={station.favicon}
            style={{
              width: 24,
              height: 24,
              marginRight: 8,
              borderRadius: '30%',
            }}
          />
        )}

        <strong>{station.name}</strong>
      </Typography>
      <Typography variant='body2' color='text.secondary'>
        {station.country} {getFlag(station.countryCode)}
        {station.codec && ` • ${station.codec.toUpperCase()}`}
        {station.bitrate > 0 && ` • ${station.bitrate} kbps`}
        {station.hls && ' • HLS'}
      </Typography>
      {station.tags && station.tags.length > 0 && (
        <Typography variant='body2' color='text.secondary'>
          {station.tags.join(' • ')}
        </Typography>
      )}
      <Button
        variant='outlined'
        size='small'
        onClick={() => onPlayStreamURL(station.urlResolved || station.url)}
        sx={{ mt: 1, mr: 1 }}
      >
        Listen
      </Button>
      {isFavourite ? (
        <IconButton
          size='small'
          color='primary'
          sx={{ mt: 1 }}
          onClick={() => onRemoveFavourites(station.id)}
        >
          <Star />
        </IconButton>
      ) : (
        <IconButton
          size='small'
          color='default'
          sx={{ mt: 1 }}
          onClick={() => onAddFavorites(station)}
        >
          <StarOutlineIcon />
        </IconButton>
      )}
    </Paper>
  );
}
