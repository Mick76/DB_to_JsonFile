#include "dbApp.h"

float
inverseLerp(float a, float b, float v) { return (v - a) / (b - a); }

template<typename T>
static T
lerp(T a, T b, float scale) {
  return static_cast<T>(static_cast<float>(a) +
                       (static_cast<float>(b) - static_cast<float>(a)) * scale);
}

namespace dbTool {

  dbApp::dbApp() {

  }

  dbApp::~dbApp() {

  }

  void 
  dbApp::run() {
    
    start();
    
    sf::Clock clock;
    while (m_window->isOpen())
    {
      float dt = clock.restart().asSeconds();
      sf::Event event;
      while (m_window->pollEvent(event))
      {
        if (event.type == sf::Event::Closed) {
          m_window->close();
        }
      }
    
      update(dt);
      render();
    }
    
    shutdown();
  }

  void 
  dbApp::start() {
    std::string database;
    cout << "database: ";
    std::cin >> database;
    m_connecter.connect(database);

    m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(m_resolution.x, m_resolution.y), "DB tool");
    m_view.reset(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(m_resolution.x, m_resolution.y)));
    m_window->setView(m_view);

    initHeatMap();
    drawHeatMap();
  }

  void
  dbApp::update(float deltaTime) {

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
    {
      if (!m_keyPressed)
      {
        m_keyPressed = true;
        m_connecter.getData(m_positions);
      }
    }
    else
    {
      m_keyPressed = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
    {
      m_connecter.disconnect();
      system("cls");
      std::string database;
      cout << "database: ";
      std::cin >> database;
      m_connecter.connect(database);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
    {
      initHeatMap();
      drawHeatMap();
    }

  }

  void 
  dbApp::render() {
    m_window->clear();
    m_window->draw(m_heatSprite);
    m_window->display();
  }

  void 
  dbApp::shutdown() {
    m_connecter.disconnect();
  }

  void 
  dbApp::initHeatMap() {

    m_normalizedMap.clear();

    m_connecter.getData(m_positions);

    std::map<sf::Vector3f, int, cmpVector3f> countingMap;
    std::map<sf::Vector3f, int, cmpVector3f> normalizedMap;

    sf::Vector3f minPoint(-1000, -1000, -1000);
    sf::Vector3f maxPoint(1000, 1000, 1000);
    sf::Vector3f scale(0.0f, 0.0f, 0.0f);

    int maxCount = 0;

    for (sf::Vector3f pos : m_positions) {
      if (countingMap.find(pos) == countingMap.end()) {
        countingMap.insert(std::make_pair(pos, 1));
      }
      else { ++countingMap[pos]; }
    }

    scale.x = static_cast<int>(std::abs(minPoint.x - maxPoint.x));
    scale.y = static_cast<int>(std::abs(minPoint.y - maxPoint.y));
    scale.z = static_cast<int>(std::abs(minPoint.z - maxPoint.z));

    for (auto pos : countingMap) {
      sf::Vector3f normalized(pos.first.x, pos.first.y, pos.first.z);

      normalized.x = (normalized.x - minPoint.x) / scale.x;
      normalized.y = (normalized.y - minPoint.y) / scale.y;
      normalized.z = (normalized.z - minPoint.z) / scale.z;

      normalizedMap.insert(std::make_pair(normalized, pos.second));

      maxCount = maxCount > pos.second ? maxCount : pos.second;
    }

    m_normalizedMap = normalizedMap;
  }

  void 
  dbApp::drawHeatMap()
  {
    m_heatMap.clear();

    //resize map
    m_heatMap.resize(m_resolution.x);
    for (int i = 0; i < m_heatMap.size(); ++i) {
      m_heatMap[i].resize(m_resolution.y);
    }

    //locate object pos and set heat to 1
    sf::Vector2i pixelPos;
    for (auto pos : m_normalizedMap) {
      pixelPos.x = pos.first.x * (m_resolution.x);
      pixelPos.y = pos.first.y * (m_resolution.y);
      if ((pixelPos.x >= 0) && (pixelPos.x < m_resolution.x) &&
        (pixelPos.y >= 0) && (pixelPos.y < m_resolution.y)) {
        m_heatMap[pixelPos.x][pixelPos.y] = pos.second;
      }
    }

    //propagate
    for (int i = 0; i < m_resolution.x; ++i) {
      for (int j = 0; j < m_resolution.y; ++j) {
        propagateInfluence(sf::Vector2i(i, j));
      }
    }
    for (int i = m_resolution.x - 1; i > -1; --i) {
      for (int j = m_resolution.y - 1; j > -1; --j) {
        propagateInfluence(sf::Vector2i(i, j));
      }
    }

    sf::Image heatImage;
    heatImage.create(m_resolution.x, m_resolution.y, sf::Color(0,0,0,0));

    int j = 0;
    sf::Vector2i pixelPosition;
    sf::Color c;
    uint32_t posX = 0;
    uint32_t posY = 0;

    sf::Color heat1(255,0,0,204);
    sf::Color heat2(255,255,0,153);
    sf::Color heat3(0,255,255,102);
    sf::Color heat4(0,0,255,76.5f);
    sf::Color heat5(0, 0, 0, 0);

    posX = 0;
    posY = 0;

    for (auto column : m_heatMap) {
      posY = 0;
      for (auto pos : column) {
        if (pos > 0.75) {
          c = BlendColor(heat2, heat1, clamp(inverseLerp(0.75, 1.5, pos), 0.0f, 1.0f));
        }
        else if (pos > 0.50) {
          c = BlendColor(heat3, heat2, inverseLerp(0.50, 0.75, pos));
        }
        else if (pos > 0.25) {
          c = BlendColor(heat4, heat3, inverseLerp(0.25, 0.50, pos));
        }
        else {
          c = BlendColor(heat5, heat4, inverseLerp(0.00, 0.25, pos));
        }

        heatImage.setPixel(posX, posY, c);
        ++posY;
      }
      ++posX;
    }

    m_heatRT.create(m_resolution.x, m_resolution.y);
    m_heatRT.update(heatImage);
    m_heatSprite.setTexture(m_heatRT);
  }

  sf::Color 
  dbApp::BlendColor(sf::Color color1, sf::Color color2, float fraction) {
    sf::Color toReturn;

    toReturn.r = static_cast<sf::Uint8>((color2.r - color1.r) * fraction + color1.r);
    toReturn.g = static_cast<sf::Uint8>((color2.g - color1.g) * fraction + color1.g);
    toReturn.b = static_cast<sf::Uint8>((color2.b - color1.b) * fraction + color1.b);
    toReturn.a = static_cast<sf::Uint8>((color2.a - color1.a) * fraction + color1.a);

    return toReturn;
  }

  void 
  dbApp::calculateInfluence(int x, int y, float& maxInf, float& minInf) {
    if ((x >= 0) && (x < m_resolution.x) && (y >= 0) && (y < m_resolution.y)) {
      float inf = getInfluence(x, y) * expf(-1 * m_decay);
      maxInf = std::max(inf, maxInf);
      minInf = std::min(inf, minInf);
    }
  }

  void
  dbApp::propagateInfluence(sf::Vector2i pos) {
    float maxInf = 0.0f;
    float minInf = 0.0f;

    //Left
    calculateInfluence(pos.x - 1, pos.y, maxInf, minInf);
    //UP Left
    calculateInfluence(pos.x - 1, pos.y + 1, maxInf, minInf);
    //Up
    calculateInfluence(pos.x, pos.y + 1, maxInf, minInf);
    //Up Right
    calculateInfluence(pos.x + 1, pos.y + 1, maxInf, minInf);
    //Right
    calculateInfluence(pos.x + 1, pos.y, maxInf, minInf);
    //Down right
    calculateInfluence(pos.x + 1, pos.y - 1, maxInf, minInf);
    //Down
    calculateInfluence(pos.x, pos.y - 1, maxInf, minInf);
    //Down Left
    calculateInfluence(pos.x - 1, pos.y - 1, maxInf, minInf);

    float newInfluence = minInf + maxInf;
    setInfluence(pos, lerp(getInfluence(pos.x, pos.y), newInfluence, m_momentum));
  }

  void
  dbApp::setInfluence(sf::Vector2i pos, float influence) {
    m_heatMap[pos.x][pos.y] = influence;
  }

  float dbApp::getInfluence(float x, float y)
  {
    return m_heatMap[x][y];
  }
}
