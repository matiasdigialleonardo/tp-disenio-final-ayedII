#include <iostream>
#include <json.hpp>
#include <algorithm>
#include <fstream>

using namespace std;
using json = nlohmann::json;

class Sensor
{
    public:
        virtual void sensate(json data) = 0;
        virtual json getData() = 0;
};

class TemperatureSensor : public Sensor
{
    private:
        float temperature;

    public:
        void sensate(json data)
        {
            temperature = data["temperature"];
        }

        json getData()
        {
            json data;
            data["temperature"] = temperature;
            return data;
        }
};

class HumiditySensor : public Sensor
{
    private:
        float humidity;

    public:
        void sensate(json data)
        {
            humidity = data["humidity"];
        }

        json getData()
        {
            json data;
            data["humidity"] = humidity;
            return data;
        }
};

class SensorFactory
{
    public:
        Sensor* getSensor(string sensorType)
        {
            if (sensorType == "TEMPERATURE")
            {
                return new TemperatureSensor();
            }
            if (sensorType == "HUMIDITY")
            {
                return new HumiditySensor();
            }

            return nullptr;
        }
};

class IMailSender
{
    public:
        virtual void addMail(string mail) = 0;
        virtual void sendMails(string message) = 0;
};

class MailSender : public IMailSender
{
    private:
        vector<string>_mails;
    public:
        void addMail(string mail)
        {
            _mails.push_back(mail);
        }
        void sendMails(string message)
        {
            for (string mail : _mails)
            {
                cout << "Receiver: " << mail << endl;
                cout << "Message: " << message << endl;
                cout << endl;
            }
        }
};

class ILogger
{
    public:
        virtual void log(json data) = 0;
};

class FileLogger : public ILogger
{
    private:
        string _fileName;
    public:
        FileLogger(){};
        FileLogger(string fileName)
        {
            _fileName = fileName;
        }
        void log(json data)
        {
            // Perform logging to a file using the provided data
            ofstream file(_fileName, ios::app);
            if (file.is_open())
            {
                file << data.dump(4);  // Write JSON data to the file
                file << endl;
                file.close();
            }
            else
            {
                cout << "Failed to open file for logging." << endl;
            }
        }
};


class WeatherManager : public MailSender, FileLogger
{
    private:
        vector<Sensor*>_sensors;
        json _data;
        FileLogger* _fileLogger;
        MailSender* _mailSender;
    public:
        WeatherManager(){};
        WeatherManager(FileLogger* fileLogger, MailSender* mailSender)
        {
            _fileLogger = fileLogger;
            _mailSender = mailSender;
        }
        ~WeatherManager(){};

        void addSensor(Sensor* sensor)
        {
            _sensors.push_back(sensor);
        }
        void sensate(json data)
        {
            for (Sensor* sensor : _sensors)
            {
                sensor->sensate(data);
            }
        }
        void log()
        {
            // To do: Rewrite to handle multiple sensors of same type.
            for (Sensor* sensor : _sensors)
            {
                json sensorData = sensor->getData();
                _data.merge_patch(sensorData);
            }

            _fileLogger->log(_data);
        }
};


int main()
{
    json data;
    data["temperature"] = 50;
    data["humidity"] = 45.5;

    SensorFactory* sensorFactory = new SensorFactory();

    Sensor* tempSensor = sensorFactory->getSensor("TEMPERATURE");
    Sensor* humiditySensor = sensorFactory->getSensor("HUMIDITY");

    FileLogger* weatherLogger = new FileLogger("weather.txt");
    MailSender* mailSender = new MailSender();

    WeatherManager* weatherManager = new WeatherManager(weatherLogger, mailSender);
    weatherManager->addSensor(tempSensor);
    weatherManager->addSensor(humiditySensor);
    weatherManager->sensate(data);

    weatherManager->log();

    delete tempSensor;
    delete humiditySensor;
    delete sensorFactory;
    delete weatherLogger;
    delete mailSender;
    delete weatherManager;

    return 0;
}