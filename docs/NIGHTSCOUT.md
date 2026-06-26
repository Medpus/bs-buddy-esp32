# Nightscout setup

BS-Buddy reads from the standard Nightscout REST API. You need two things in
`include/secrets.h`: your site URL and (usually) a read-only token.

## URL

```c
#define NS_URL "https://your-site.example.com"   // no trailing slash
```

## Token (recommended)

If your site is locked down (`AUTH_DEFAULT_ROLES=denied`), create a **read-only**
access token (don't use your `API_SECRET`):

1. Open your Nightscout site → hamburger menu → **Admin Tools**.
2. Under **Subjects**, click **Add new Subject**.
3. Name it e.g. `bsbuddy`, set **Roles** to `readable`, **Save**.
4. Copy the generated token; it looks like `bsbuddy-1a2b3c4d5e6f7a8b`.
5. Paste it into `secrets.h`:
   ```c
   #define NS_TOKEN "bsbuddy-1a2b3c4d5e6f7a8b"
   ```

If your site is public (`AUTH_DEFAULT_ROLES=readable`), you can leave
`NS_TOKEN ""`, but creating a `readable` token is still good practice.

## What it fetches

`GET {NS_URL}/api/v1/entries.json?count={N}&token={NS_TOKEN}`

`N` is `BSB_HISTORY_N`, enough recent readings (~5 min apart) to draw the bottom
sparkline. From the response (newest-first):

- The **latest** entry is the current value. `sgv` is always in **mg/dL** →
  converted to mmol/L on the device (÷ 18.01559).
- `direction` on the latest entry → trend arrow.
- `delta` is computed locally from the two most recent readings.
- All `sgv` values feed the bottom **sparkline**.

Polling is every 60 s by default (`BSB_POLL_INTERVAL_MS` in `src/core/config.h`); CGM
data only updates ~every 5 min, so there's no point polling faster.

## Verifying from a computer

You can sanity-check your URL + token in a browser or curl:

```bash
curl "https://your-site.example.com/api/v1/entries.json?count=1&token=bsbuddy-xxxx"
```

You should get a JSON array with an `sgv` field. A `401`/`403` means the token is
missing/wrong; an empty array means no recent CGM data.

## Trend directions

| Nightscout `direction` | Arrow | Meaning |
|---|---|---|
| `DoubleUp`      | ↑↑ | rising very fast |
| `SingleUp`      | ↑  | rising fast |
| `FortyFiveUp`   | ↗  | rising |
| `Flat`          | →  | steady |
| `FortyFiveDown` | ↘  | falling |
| `SingleDown`    | ↓  | falling fast |
| `DoubleDown`    | ↓↓ | falling very fast |
