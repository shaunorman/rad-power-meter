# Rad Power Meter

I’ve always been fascinated by statistics, especially live statistics about events happening right now. I was also curious as to what was causing our high power bills so I  had an idea to somehow log our power usage throughout the day to see what exactly is causing these high bills.  
I like to tinker with absolutely anything, plus I also know how to write software as that’s my job so I thought why not combine the two in this case and build a device to give me instant power usage for our home.
I searched a bit a pre-built product and I found a few but nothing let me export or save any of the data, and if it did, it wasn’t an easy process so I threw that idea in the trash, but not without taking a few ideas with me.
Some of the meters out there required a qualified electrician to install CT (Current Transformer) Meters on each of the wires (in our case 3, due to 3 phase) at the back of the power meter. That pretty blew my budget of $0 to a few hundred so there goes that idea. There were these fancy 3G connected ones which somehow integrated into the power meter but cost a few hundred dollars on their own, not including installation but I finally went back to the easiest possible thing there was, monitoring a light turning on and off.
How does that work you ask? Well, for every watt that is consumed through the meter an LED flashes on and off, the more power you use, the faster it flashes so if you can sense the light flashing on and off accurately, you can determine your power usage. And it just so happens we have one of the newer power meters which has one of these lights so it should be a simple task to read this.
[insert image of power meter from western power]
https://westernpower.com.au/media/1553/three-phase-ami-smartmeter-fact-sheet.pdf
Now, I’ve made a few little IoT devices before that involved things like monitoring the pool levels, temperatures, outdoor temperature and humidity, rainfall etc. so I pretty much had all the hardware laying around, so all it up it just cost me time.
The main brain of this project I decided to use the NodeMCU platform which has the ESP8266 WiFi System on a chip, basically it’s a small compact cheap arduino clone with wifi for around $5 AUD
https://en.wikipedia.org/wiki/NodeMCU
This was then mounted to a prototype board with a 4 pin female plug and mounted on the window frame within the garage. Now, none of this is even close to the power meter because I’ve had to place the computing unit somewhere where I could get a WiFi signal and stay connected to my router. 
https://i.imgur.com/hlmIRxH.jpg
So, from that plug runs a few meters of Cat6e cable to another plug mounted on another prototype board along with a LDR module. These can be seen here side by side with the LDR (Light dependent resistor) module [https://www.kitronik.co.uk/blog/how-an-ldr-light-dependent-resistor-works] in the middle. 
https://i.imgur.com/gWMi40i.jpg
 
The LDR sensor is then placed over the LED on the meter with a huge chunk of Blu Tack which serves two purposes. The first is to hold the sensor in the correct position right in front of the LED and the second is to block out as much external light as possible so we don’t get anything interfering with the sensor. 
https://i.imgur.com/e7nomO9.jpg
The LDR and it’s connected module is what is used to tell me when the LED light turns on and off. When the LED is on, the resistance in the sensor becomes low and once it goes past the level I have selected on the module a switch internally turns on and supplies voltage over the digital pin. Back on the other end the NodeMCU has been setup to listen for the exact moment this pin turns on and off and I record this instant at 1/1000th of a second accuracy.
When the device is booted it up instantly connects to the WiFi router and then connects to an MQTT server so that every instant the light turns on a message is published to the service with the exact time the LED turns on. Each one of these messages is then processed via a NodeJS server and the data then fed into a Postgres server recording every single watt used in my home and the instant it happened.
You might then ask, now what? Well, now that we have all this data recorded 24/7 down to the millisecond we can now push this data into any statistics software, even Excel to get a quick understanding of when the peak power usage occurs and even work out an very close estimate of the next power bill.
Not only looking into power consumption by the month, day, or even hour but as every watt used is sent instantly to the server we can calculate the exact power being used at any instant with a simple formula.
60mins * 60 seconds = [seconds in 1 hour]
([current watt time in milliseconds] - [previous watt time in milliseconds]) / 1000 = [time to use a watt in seconds]
[time to use a watt] / [seconds in 1 hour] = [watts per hour]
[watts per hour] / 1000 = [KW per hour]
This data along with any other I decide to calculate can then be fed into systems like the Apple Home Kit or even Google Assistant so you can ask them 
“Hey Siri, How much power have we used today?”
“OK Google, how much power is being used right now?”
Another question my partner brought up is “How accurate is it?” We’ll, I had already prototyped another unit so I could take it around the house and turn devices on and off to see how well it performed. This was a tiny little OLED screen hooked up to another NodeMCU and that simply subscribed to a /current-power-usage topic on the MQTT server, every watt submitted to it I applied the formula above then spat the information out to the little unit and it displayed it on the screen. I also logged it out on the server.

[Insert picture of OLED screen with current power usage on it]

I turned of aircon, fridges and anything else that would cause a large surge in the power and just left all the other devices on standby and plugged in a 2kw kettle and the results amazed me

KW/h: 0.2613
KW/h: 0.2699
<kettle on>
KW/h: 1.7845
KW/h: 2.2598
KW/h: 2.2542
KW/h: 2.2570
KW/h: 2.2598
KW/h: 2.2556
KW/h: 2.2570
KW/h: 2.2556
KW/h: 2.2613
KW/h: 2.2598
KW/h: 2.2584
KW/h: 2.2514
<kettle off>
KW/h: 0.3439
KW/h: 0.2713

Once it was turned on it jumped almost exactly 2kw, pretty damn spot on! So, I went to the bathroom which had two 10Watt LED lights in there, and it jumped exactly 20W and fell 20W every time I turned the light switch on and off. I tried a few more things that I thought would draw the same power no matter what and I came up with consistant results so I could put that question to bed thats for sure.

So what’s next for this little project?  Well, hardware wise I’ve just ordered a few little solar panels which I will be hooking up to a Lithium Polymer battery via some circuitry. Funnily enough, even though all the houses power goes through the thing I’m recording there isn’t a power point within 10 metres of the box so at present I’m running it via an extension cord and old iphone charger.

Also, to place a few CT clamps on individual devices power cords around the house to measure the consumption of these devices, especially when in standby mode and not being used for quite some time.

Software wise, all this data being fed into my home hub so all power usage can be looked at history and compared with previous bills, outside weather also see the affect when new appliances are introduced into the house.

My code for this little project can be found here on my Github page.
https://github.com/shaunorman/rad-power-meter
And the file which does pretty much everything on the NodeMCU: https://github.com/shaunorman/rad-power-meter/blob/master/src/main.cpp

[insert some images at bottom or in article from: https://imgur.com/a/1hJhoq1
]




