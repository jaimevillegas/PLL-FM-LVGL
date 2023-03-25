#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9488 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;

  // lgfx::Touch_XPT2046          _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      // cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.dma_channel = 1;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 23;
      cfg.pin_miso = 19;
      cfg.pin_dc = 16;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 5;
      cfg.pin_rst = 17;
      cfg.pin_busy = -1;

      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = 21;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    {
      // auto cfg = _touch_instance.config();

      // cfg.x_min = 0;
      // cfg.x_max = 239;
      // cfg.y_min = 0;
      // cfg.y_max = 479;
      // cfg.pin_int = -1;
      // cfg.bus_shared = true;
      // cfg.offset_rotation = 0;

      // // SPI接続の場合
      // cfg.spi_host = VSPI_HOST; // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
      // cfg.freq = 1000000;       // SPIクロックを設定
      // cfg.pin_sclk = 18;        // SCLKが接続されているピン番号
      // cfg.pin_mosi = 23;        // MOSIが接続されているピン番号
      // cfg.pin_miso = 19;        // MISOが接続されているピン番号
      // cfg.pin_cs = 5;           //   CSが接続されているピン番号

      // _touch_instance.config(cfg);
      // _panel_instance.setTouch(&_touch_instance); // タッチスクリーンをパネルにセットします。
    }
    //*/

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

// 準備したクラスのインスタンスを作成します。