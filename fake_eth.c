#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/errno.h>
#include<linux/init.h>
#include<linux/netdevice.h>
#include<linux/etherdevice.h>
#include<linux/ethtool.h>
#include<linux/skbuff.h>
#include<linux/slab.h>
#include<linux/of.h>
#include<linux/platform_device.h>

struct eth_struct {
    int bar;
    int foo;
    struct net_device *dummy_ndev;
} ;

static int fake_eth_open(struct net_device *dev) {
    pr_info("fake_eth_device open called\n");
    netif_start_queue(dev);
    return 0;
}

static int fake_eth_release(struct net_device *dev) {
    pr_warn("fake_eth_device release called\n") ;
    netif_stop_queue(dev) ;
    return 0;

}

static int fake_eth_xmit(struct sk_buff *skb, struct net_device *ndev) {
    pr_info("dummy xmit called\n");
    ndev->stats.tx_bytes += skb->len ;
    ndev->stats.tx_packets++ ;

    skb_tx_timestamp(skb);
    dev_kfree_skb(skb);
    return NETDEV_TX_OK ;
}

static int fake_eth_init(struct net_device *dev) {
    pr_warn("fake eth device init done\n") ;
    return 0;

}

static const struct net_device_ops my_netdev_ops = {

    .ndo_init = fake_eth_init,
    .ndo_open = fake_eth_open,
    .ndo_stop = fake_eth_release,
    .ndo_start_xmit = fake_eth_xmit,
    .ndo_validate_addr = eth_validate_addr,
    .ndo_validate_addr = eth_validate_addr,
};

static const struct of_device_id  fake_eth_dt_ids[] = {
    {.compatible = "packt, fake-eth", },
    {  }
};


static int fake_eth_probe(struct platform_device *pdev) {
    
    int ret ;
    struct eth_struct *priv;
    struct net_device *dummy_ndev ;
    
    priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);

    dummy_ndev = alloc_etherdev(sizeof(struct eth_struct)) ;

    dummy_ndev->if_port = IF_PORT_10BASET;
    dummy_ndev->netdev_ops = &my_netdev_ops ;

    ret = register_netdev(dummy_ndev);
    
    if(ret) {
        pr_info("dumm net dev: Error %d initialization card\n", ret);
        return ret ;
    } //if 

    priv->dummy_ndev = dummy_ndev;
    platform_set_drvdata(pdev, priv);
    return 0;

}

static int fake_eth_remove(struct platform_device *pdev) {
    struct eth_struct *priv ;
    priv = platform_get_drvdata(pdev);
    pr_info("Cleaning up the module\n");
    unregister_netdev(priv->dummy_ndev);
    free_netdev(priv->dummy_ndev) ;

    return 0 ;
}

static struct platform_driver mypdrv = {
    .probe = fake_eth_probe,
    .remove = fake_eth_remove,
        .driver = {
        .name = "fake-eth",
        .of_match_table = of_match_ptr(fake_eth_dt_ids),
        .owner = THIS_MODULE,
    },  
};

module_platform_driver(mypdrv);

MODULE_LICENSE("GPL") ;
MODULE_AUTHOR("Fomin Nikita");
MODULE_DESCRIPTION("Fake ethernet driver");
