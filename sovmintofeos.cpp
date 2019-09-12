#include <sovmintofeos.hpp>


namespace eosio {

void token::create( name   issuer,
                    asset  maximum_supply )
  {
    require_auth( _self );

    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) 
      {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
       s.burn_time      = now();
      });
  }


void token::issue( name to, asset quantity, string memo )
  {
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s )
      {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );

    if( to != st.issuer ) 
      {
        SEND_INLINE_ACTION( *this, transfer, { {st.issuer, "active"_n} },
                          { st.issuer, to, quantity, memo }
        );
      }
  }

void token::retire( asset quantity, string memo )
  {
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) 
      {
       s.supply -= quantity;
      });

    sub_balance( st.issuer, quantity );
    
  }



void token::selfburn( name account, asset quantity, string memo )
  {
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( account );
    require_auth(name{"themintofeos"});
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) 
      {
       s.supply -= quantity;
      });

    sub_balance( account, quantity );
    
  }



void token::transfer( name    from,
                      name    to,
                      asset   quantity,
                      string  memo )
  {
    
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "to account does not exist");
    
    auto sym = quantity.symbol.code();
    stats statstable( _self, sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );
    require_recipient(name{"themintofeos"});

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity of atleast 0.0001 units" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;
    auto circulating = get_supply(_self, sym);
    
    uint64_t deflation_factor = 0;
    uint64_t stage;
      
    if ((circulating.amount/10000) >= 990000000) //divide by 10000 because sym precision multiplies the int by 10^(sym precision amount)
      { 
        stage = 1;
        deflation_factor = 5;//is actually 0.05%
      }
    else if ((circulating.amount/10000) >= 980000000)
      {
        stage = 2;
        deflation_factor = 10;
      }
    else if ((circulating.amount/10000) >= 970000000)
      {
        stage = 3;
        deflation_factor = 15;
      }
    else if ((circulating.amount/10000) >= 960000000)
      {
        stage = 4;
        deflation_factor = 20;
      }
    else if ((circulating.amount/10000) >= 950000000)
      {
        stage = 5;
        deflation_factor = 25;
      }
    else if ((circulating.amount/10000) >= 940000000)
      {
        stage = 6;
        deflation_factor = 30;
      }
    else if ((circulating.amount/10000) >= 930000000)
      {
        stage = 7;
        deflation_factor = 35;
      }
    else if ((circulating.amount/10000) >= 825000000)
      {
        stage = 8;
        deflation_factor = 50;
      }
    else if ((circulating.amount/10000) >= 720000000)
      {
        stage = 9;
        deflation_factor = 60;
      }
    else if ((circulating.amount/10000) >= 615000000)
      {
        stage = 10;
        deflation_factor = 70;
      }
    else if ((circulating.amount/10000) >= 510000000)
      {
        stage = 11;
        deflation_factor = 80;
      }
    else if ((circulating.amount/10000) >= 405000000)
      {
        stage = 12;
        deflation_factor = 90;
      }
    else if ((circulating.amount/10000) >= 300000000)
      {
        stage = 13;
        deflation_factor = 100;
      }
    else if ((circulating.amount/10000) >= 195000000)
      {
        stage = 14;
        deflation_factor = 125;
      }
    else if ((circulating.amount/10000) >= 166000000)
      {
        stage = 15;
        deflation_factor = 100;
      }
    else if ((circulating.amount/10000) >= 137000000)
      {
        stage = 16;
        deflation_factor = 75;
      }
    else if ((circulating.amount/10000) >= 108000000)
      {
        stage = 17;
        deflation_factor = 50;
      }
    else if ((circulating.amount/10000) >= 79000000)
      {
        stage = 18;
        deflation_factor = 25;
      }
    else if ((circulating.amount/10000) >= 50000000)
      {
        stage = 19;
        deflation_factor = 15;
      }
    else if ((circulating.amount/10000) >= 21000000)
      {
        stage = 20;
        deflation_factor = 5;
      }
      
    else
      {
         stage = 21;
        deflation_factor = 0;
      }
  
    asset burned = (quantity*deflation_factor/10000);
    
    if ((burned.amount == 0)&&stage!=21)
      { //round burn to 0.0001 unless at stage 21
        burned.amount = 1;
      }
    
  
    //Make Sender Pay Burn, most of extra code is to generate error message)
    
    quantity = quantity + burned;
    accounts from_acnts( _self, from.value );
    const auto& frombal = from_acnts.get( quantity.symbol.code().raw(), "no balance object found" );
    
    double frombaldec = frombal.balance.amount;
    frombaldec = frombaldec / 10000;
    frombaldec = round(frombaldec*10000);
    frombaldec = frombaldec /10000;
    
    double deflationdec = deflation_factor;
    deflationdec = deflationdec / 10000;
    double maxsend = frombaldec / (1+ deflationdec);
    maxsend = round(maxsend*10000);
    maxsend = (maxsend) /10000;
    
    std::string bal_error_msg = ("Transfer failed. Your balance is: " +  std::to_string(frombaldec) + ", you can send a max amount of: " + std::to_string(maxsend));
    check( frombal.balance.amount >= quantity.amount, bal_error_msg );
    
    
    sub_balance( from, quantity );
    
    if (stage != 21)
      { // no burning at 21
        
        quantity.amount = (quantity.amount- burned.amount); // remove the burn from the send amount
        
        
        statstable.modify( st, same_payer, [&]( auto& s )
          {
            s.supply -= burned;
          });
          
      }
    
    add_balance( to, quantity, payer );
  }
  

void token::xtransfer( name    from,
                      name    to,
                      asset   quantity,
                      string  memo )
  {
    
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "to account does not exist");
    
    auto sym = quantity.symbol.code();
    stats statstable( _self, sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );
    require_recipient(name{"themintofeos"});

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity of atleast 0.0001 units" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;
    auto circulating = get_supply(_self, sym);
    
    uint64_t deflation_factor = 0;
    uint64_t stage;
      
    if ((circulating.amount/10000) >= 990000000) //divide by 10000 because sym precision multiplies the int by 10^(sym precision amount)
      { 
        stage = 1;
        deflation_factor = 5;//is actually 0.05%
      }
    else if ((circulating.amount/10000) >= 980000000)
      {
        stage = 2;
        deflation_factor = 10;
      }
    else if ((circulating.amount/10000) >= 970000000)
      {
        stage = 3;
        deflation_factor = 15;
      }
    else if ((circulating.amount/10000) >= 960000000)
      {
        stage = 4;
        deflation_factor = 20;
      }
    else if ((circulating.amount/10000) >= 950000000)
      {
        stage = 5;
        deflation_factor = 25;
      }
    else if ((circulating.amount/10000) >= 940000000)
      {
        stage = 6;
        deflation_factor = 30;
      }
    else if ((circulating.amount/10000) >= 930000000)
      {
        stage = 7;
        deflation_factor = 35;
      }
    else if ((circulating.amount/10000) >= 825000000)
      {
        stage = 8;
        deflation_factor = 50;
      }
    else if ((circulating.amount/10000) >= 720000000)
      {
        stage = 9;
        deflation_factor = 60;
      }
    else if ((circulating.amount/10000) >= 615000000)
      {
        stage = 10;
        deflation_factor = 70;
      }
    else if ((circulating.amount/10000) >= 510000000)
      {
        stage = 11;
        deflation_factor = 80;
      }
    else if ((circulating.amount/10000) >= 405000000)
      {
        stage = 12;
        deflation_factor = 90;
      }
    else if ((circulating.amount/10000) >= 300000000)
      {
        stage = 13;
        deflation_factor = 100;
      }
    else if ((circulating.amount/10000) >= 195000000)
      {
        stage = 14;
        deflation_factor = 125;
      }
    else if ((circulating.amount/10000) >= 166000000)
      {
        stage = 15;
        deflation_factor = 100;
      }
    else if ((circulating.amount/10000) >= 137000000)
      {
        stage = 16;
        deflation_factor = 75;
      }
    else if ((circulating.amount/10000) >= 108000000)
      {
        stage = 17;
        deflation_factor = 50;
      }
    else if ((circulating.amount/10000) >= 79000000)
      {
        stage = 18;
        deflation_factor = 25;
      }
    else if ((circulating.amount/10000) >= 50000000)
      {
        stage = 19;
        deflation_factor = 15;
      }
    else if ((circulating.amount/10000) >= 21000000)
      {
        stage = 20;
        deflation_factor = 5;
      }
      
    else
      {
         stage = 21;
        deflation_factor = 0;
      }
  
    asset burned = (quantity*deflation_factor/10000);
    
    if ((burned.amount == 0)&&stage!=21)
      { //round burn to 0.0001 unless at stage 21
        burned.amount = 1;
      }
    
    float deflation_percent = (deflation_factor);
    deflation_percent = deflation_percent/100;
    
    
    
    sub_balance( from, quantity );
    
    if (stage != 21)
      { // no burning at 21
        
        quantity.amount = (quantity.amount- burned.amount); // remove the burn from the send amount
       
        
        statstable.modify( st, same_payer, [&]( auto& s )
          {
            s.supply -= burned;
          });
          
      }
    
    add_balance( to, quantity, payer );
  }
  



void token::sub_balance( name owner, asset value ) 
  {
    /**
    this function has been modified from eosio.token to implement store/withdraw system
    blocks transacting with stored balance and balances that are undergoing the three day clear period
    **/
    uint32_t time_now = now();
    uint32_t three_days_time = 259200;//3 days in seconds
    accounts from_acnts( _self, owner.value );

    const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
    check( from.balance.amount >= value.amount, "overdrawn balance" );

    from_acnts.modify( from, owner, [&]( auto& a ) 
      {
         
        if(time_now<=(a.unstake_time+three_days_time)) //checks if anything is in process of unstaking, will stop transfer of tokens in process of unstaking
          {
            uint32_t time_left = round(((a.unstake_time + three_days_time) - time_now)/60); // for error msg display only
            std::string errormsg = "Cannot send nor burn stored tokens. Tokens in withdraw will be availible in " + std::to_string(time_left) + " minute(s)";
            check(value <= (a.balance-(a.unstaking+a.storebalance)),errormsg);
          }
       
        else 
          {
           check(value <= (a.balance-a.storebalance),"Cannot send nor burn stored tokens");//there are no funds in process of unstaking, only stop transfer of stored funds
          }
         
         a.balance -= value;
      });
  }

void token::add_balance( name owner, asset value, name ram_payer )
  {
    accounts to_acnts( _self, owner.value );
    auto to = to_acnts.find( value.symbol.code().raw() );
    
    if( to == to_acnts.end() ) 
      {
        to_acnts.emplace( ram_payer, [&]( auto& a )
          {
            a.balance = value;
            a.storebalance = (value-value); //initialiazation without needing to specify asset symbol or precision   
            a.unstaking = (value-value);     
            a.unstake_time = 0;   
        
          });
      } 
    else 
      {
        to_acnts.modify( to, same_payer, [&]( auto& a ) 
          {
            a.balance += value;
          });
      }
    }




void token::airgrab2( name owner, asset value)
  {
    
    // all genesis accounts can airgrab 5000 SOV, only once
    
    
    require_auth( owner );
    require_recipient( owner );
    
    check(value.amount == 50000000, "Invalid amount");
    
    auto sym = value.symbol.code();
    stats statstable( _self, sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    check( value.is_valid(), "invalid quantity" );
    check( value.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( value.symbol.is_valid(), "invalid symbol name" );
    
    uint64_t c_time =round((get_account_creation_time(owner.value)/1000/1000)); //find time of account creation
    uint64_t genesis_time = 1528553625; // genesis account creation time 
    check(c_time <= genesis_time, "Account created after EOS mainnet launch, not elligible");                     
    
    
    signups list(_self, _self.value); 
    auto itr = list.find(owner.value);
    check(itr == list.end(), "You have already claimed airgrab");
  
    
    accounts to_acnts( _self, owner.value );
    auto to = to_acnts.find( value.symbol.code().raw() );
  
   //if account has signed up, then see if already owns SOV, if not, have them pay ram for the table  
    
     if(itr == list.end()  )
      {
        if(to == to_acnts.end())
          {
            to_acnts.emplace( owner, [&]( auto& a )
              {
                a.balance = value-value;
                a.storebalance = (value-value); //initialiazation without needing to specify asset symbol or precision   
                a.unstaking = (value-value);     
                a.unstake_time = 0;   
        
              });
          }
      
        
        //send SOV from the contract to the claimer 
        action(permission_level{_self, "active"_n}, "sovmintofeos"_n, "xtransfer"_n, 
        std::make_tuple( _self,owner, value, std::string("Airgrab has been sent"))).send();
      
        
        list.emplace( owner, [&]( auto& a )
          {
            a.account = owner;
          });
    
      }
  }




void token::airburn(name account, asset quantity)
  {
   
    //Burn 7.5 million SOV inside contract account once a week
    
    auto burnnumber = (75000000000);
    check((quantity.amount == (burnnumber)),"Amount must equal to 7.5 million");
  
  
    check((now()>=1567296000),"Airburn does not start until September 1st, 2019");
    require_auth(account);
 
    auto sym = quantity.symbol.code();
    stats statstable( _self, sym.raw() );
    const auto& st = statstable.get( sym.raw() );
  
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check((quantity.amount == (burnnumber)),"Amount must equal to 7.5 million");
  
    uint32_t time_now = now();
    uint32_t week_of_time = 604800; //seconds in weeks
  
  
    statstable.modify( st, same_payer, [&]( auto& s ) 
      
      {
      
        if ((s.burn_time + week_of_time) >= time_now) //check burn availibility and produce error msg if not ready
          {
            uint32_t time_left = round(((s.burn_time + week_of_time) - time_now)/60);
            std::string errormsg = "Airburn not avalible for " + std::to_string(time_left) + " minutes";
            check(time_now >=(s.burn_time + week_of_time), errormsg);
          }
          
          
        accounts contract_acnt( _self, _self.value ); // if less than 7.5 mill remaining, set burn equal to contract balance
        auto to2 = contract_acnt.find( quantity.symbol.code().raw() );
        contract_acnt.modify( to2, same_payer, [&]( auto& a )
              {
                if (a.balance < quantity)
                  {
                    quantity = a.balance;
                  }
                
        
              });
      
        s.burn_time = time_now;
        action(permission_level{_self, "active"_n}, "sovmintofeos"_n, "retire"_n, 
        std::make_tuple( quantity, std::string("Burning Complete"))).send();
      
        
      });
    
  }
  
void token::store(name account, asset value)
  {
      //stake tokens so that they cannot be transferred, no other purpose
      require_auth(account);
      require_recipient(account);
    
      auto sym = value.symbol.code();
      stats statstable( _self, sym.raw() );
      const auto& st = statstable.get( sym.raw() );

      check( value.is_valid(), "invalid quantity" );
      check( value.symbol == st.supply.symbol, "symbol precision mismatch" );
      check( value.symbol.is_valid(), "invalid symbol name" );
    
      accounts to_acnts( _self, account.value );
      auto to = to_acnts.find( value.symbol.code().raw() );
    
      to_acnts.modify( to, same_payer, [&]( auto& a )
        {
          eosio::check((value <= (a.balance-a.storebalance) ), "Cannot Store more than your unstored balance");
          a.storebalance += value;
        });
    
    }
  
  
void token::withdraw(name account, asset value)
  {
      /**
      unstake tokens, needs three days to unstake
      note: unstaking is a dummy variable and should not be used by frontend apps to display unstaking amount
      is only valid as unstaking amount if current time is less than three days from unstake time
      **/
      require_auth(account);
      require_recipient(account);
    
      auto sym = value.symbol.code();
      stats statstable( _self, sym.raw() );
      const auto& st = statstable.get( sym.raw() );

      check( value.is_valid(), "invalid quantity" );
      check( value.symbol == st.supply.symbol, "symbol precision mismatch" );
      check( value.symbol.is_valid(), "invalid symbol name" );
    
      uint32_t time_now = now();
      uint32_t three_days_time = 259200; // 3 days in sec
    
      accounts to_acnts( _self, account.value );
      auto to = to_acnts.find( value.symbol.code().raw() );
   
      to_acnts.modify( to, same_payer, [&]( auto& a ) 
        {
        
          check((value <= a.storebalance  ), "Cannot withdraw more than your stored balance");
          a.storebalance -= value;
        
          if(time_now<= (three_days_time + a.unstake_time))
            {
              a.unstaking += value;
            }
          else
            {
              a.unstaking = value;
            }
          
          a.unstake_time = time_now;
        
        });
    
    }
  
 
void token::close(name owner, asset value )
  {
      require_auth( owner );
      
      accounts acnts( _self, owner.value );
      auto it = acnts.find( value.symbol.code().raw() );
      
      check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
      check( it->balance.amount == 0.0000, "Cannot close because the balance is not zero." );
      acnts.erase( it );
    }
  
 


} /// namespace eosio

