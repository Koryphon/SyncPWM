/*
 * SyncPWM library.
 *
 * Copyright Jean-Luc Béchennec 2015. Based on a design of Pierre Molinaro
 * 
 * This library allows to use a PWM which is synchronized with the same
 * PWM of other Arduino.
 *
 * Currently on ATMega328 timer2 is used so the PWM output is on pin 3
 *
 * This software is distributed under the GNU Public Licence v2 (GPLv2)
 *
 * Please read the LICENCE file
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#include "Arduino.h"
 
class SyncPWM
{
    static byte syncPWMState;
    
    /*
     * Program timer2 to start the PWM
     */
    void startPWM();
    
  public:
    /*
     * beginMasterClock starts the synchronization signal on pin 3
     */
    void beginMasterClock();
    
    /*
     * begin starts the synchronized PWM on pin 3. By default, the PWM
     * is initialized at 0. The argument gives the synchronization pin
     * where the signal coming from the master is received
     */
    void begin(const byte pin);
    
    /*
     * analogWrite changed the value of the PWM
     */
    void analogWrite(byte value);
    
    /*
     * displayPortAddrAndMask displays the port address and the bit mask
     * used for the sync input pin
     */
    void displayPortAddrAndMask();
};
